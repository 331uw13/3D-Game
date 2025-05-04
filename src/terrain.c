#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "terrain.h"
#include "state/state.h"
#include "state/state_render.h"

#include "util.h"

#include <rlgl.h>


// NOTE: X and Z must be set accordingly with terrain X,Z positions and scaling.
static size_t get_heightmap_index(struct terrain_t* terrain, float x, float z) {

    int r_x = round(x);
    int r_z = round(z);

    long int i = (r_z * terrain->heightmap.size + r_x);

    i = (i < 0) ? 0 : (i > (long int)terrain->heightmap.total_size) ? 0 : i;
    return (size_t)i;
}

float get_heightmap_value(struct terrain_t* terrain, float x, float z) {
    size_t index = get_heightmap_index(terrain, x, z);
    return terrain->heightmap.data[index];
}




RayCollision raycast_terrain(struct terrain_t* terrain, float x, float z) {

    x += -terrain->transform.m12;
    z += -terrain->transform.m14;


    Ray ray = (Ray) {
        // ray position 
        (Vector3) { x, terrain->highest_point, z }, 
        
        // set ray direction pointing straight down
        (Vector3) { 0.0, -1.0, 0.0 } 
    };

 
    // get 4 closest triangles.
    struct triangle2x_t tr[4];

    // scale x and z back to sensible coordinates for 'get_heightmap_index'.
    x /= terrain->scaling;
    z /= terrain->scaling;

    // left top
    tr[0] = terrain->triangle_lookup[get_heightmap_index(terrain, x, z)];
    
    // right top
    tr[1] = terrain->triangle_lookup[get_heightmap_index(terrain, x-1, z)];
    
    // right bottom
    tr[2] = terrain->triangle_lookup[get_heightmap_index(terrain, x-1, z-1)];
    
    // left bottom
    tr[3] = terrain->triangle_lookup[get_heightmap_index(terrain, x, z-1)];
  

    // check each ones collision with ray.

    RayCollision mesh_hit_info = { 0 };

    for(int i = 0; i < 4; i++) {

        // check triangle A
        mesh_hit_info = GetRayCollisionTriangle(ray, tr[i].a0, tr[i].a1, tr[i].a2);

        if(!mesh_hit_info.hit) { // A was not hit check B
            mesh_hit_info = GetRayCollisionTriangle(ray, tr[i].b0, tr[i].b1, tr[i].b2);
        }

        if(mesh_hit_info.hit) {
            mesh_hit_info.point.x = x;
            mesh_hit_info.point.z = z;
            break;
        }
    }


    return mesh_hit_info;
}

Matrix get_rotation_to_surface(struct terrain_t* terrain, float x, float z, RayCollision* ray_out) {
    
    RayCollision t_hit = raycast_terrain(terrain, x, z);

    Vector3 up = (Vector3){ 0.0, 1.0, 0.0};
    Vector3 axis = Vector3CrossProduct(up, t_hit.normal);

    if(ray_out) {
        *ray_out = t_hit;
    }

    return MatrixRotateXYZ((Vector3){ axis.x, 0.0, axis.z });
}

static void render_loading_info(struct state_t* gst, const char* text, int step, int max_steps) {
     
    BeginDrawing();
    {
        ClearBackground((Color){ 10, 10, 10, 255 });

        float Y = gst->screen_size.y / 2;
        DrawTextEx(
                gst->font,
                text,
                (Vector2){ 200, Y },
                25, // Font size.
                FONT_SPACING,
                (Color){ 200, 200, 200, 255 }
                );

        DrawTextEx(
                gst->font,
                TextFormat("%0.0f%%", map(step, 0, max_steps, 0, 100)),
                (Vector2){ 100, Y+5 },
                20, // Font size.
                FONT_SPACING,
                (Color){ 150, 150, 150, 255 }
                );


        const float max_width = 800;
        const float max_height = 30;
        const float padding = 5.0;
        const float bar_y = Y+50;
        DrawRectangle(100, bar_y, max_width, max_height, (Color){ 30, 30, 30, 255 });

        Color bar_color = (Color){ 50, 200, 60, 255 };
        float bar_width = map(step, 0, max_steps, 0, max_width-padding);
        DrawRectangle(100+padding, bar_y+padding, 
                bar_width, max_height-padding*2,
                bar_color);

    }
    EndDrawing();
   
}

static void _load_terrain_chunks(struct state_t* gst, struct terrain_t* terrain) {
    
    int terrain_size = terrain->heightmap.size;

    terrain->chunk_size = CHUNK_SIZE;
    terrain->num_chunks 
        = (terrain_size / terrain->chunk_size)
        * (terrain_size / terrain->chunk_size);


    printf(" Terrain size x*z: %i\n", terrain_size * terrain_size);
    printf(" Chunk size x*z: %i\n", terrain->chunk_size * terrain->chunk_size);
    printf(" Num chunks: %li\n", terrain->num_chunks);

    if(terrain->chunks) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Terrain chunks already loaded.\033[0m\n",
                __func__);
        return;
    }

    terrain->chunks = malloc(terrain->num_chunks * sizeof *terrain->chunks);
    if(!terrain->chunks) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (malloc) %s\033[0m\n",
                __func__, strerror(errno));
        return;
    }

    memset(terrain->chunks, 0, terrain->num_chunks * sizeof *terrain->chunks);


    const int chunk_triangle_count = 2*((terrain->chunk_size) * (terrain->chunk_size));

    printf(" Chunk triangle count: %i\n", chunk_triangle_count);
    int chunk_x = 0;
    int chunk_z = 0;
    

    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        *chunk = (struct chunk_t) { 0 };
        chunk->index = i;
  
        
        render_loading_info(gst, "Loading chunks",
                i, terrain->num_chunks);
       
        load_chunk(gst, terrain, chunk, chunk_x, chunk_z, chunk_triangle_count);
        
        chunk_x += terrain->chunk_size;
        if(chunk_x >= terrain_size) {
            chunk_x = 0;
            chunk_z += terrain->chunk_size;
        }

    }
}

// Write all constant information about grass blades
// for grass data ssbo (shader storage buffer object).
void write_terrain_grass_positions(struct state_t* gst, struct terrain_t* terrain) {
    if((gst->init_flags & INITFLG_GRASSDATA)) {
        fprintf(stderr, "\033[35m(WARNING) '%s': Trying to overwrite grass data\033[0m\n",
                __func__);
        return;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[GRASSDATA_SSBO]);
    size_t grasspos_index = 0;


    float time_start = GetTime();
   
    struct grassdata_t {
        float position[4];

        float _reserved0[
              4   // Settings.
            + 3*4 // Rotation (mat3x4)
        ];
    };

    const size_t data_size = 
        (terrain->num_chunks * terrain->grass_instances_perchunk)
        * (sizeof(struct grassdata_t));

    struct grassdata_t* data = malloc(data_size);


    //shader_setu_float(gst, CHUNK_FORCETEX_SHADER, U_GRASS_SPACING, &terrain->grass_spacing);
   
    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        chunk->grass_baseindex = grasspos_index;


        render_loading_info(gst, "Loading grass", i, terrain->num_chunks);

        // Making this loop backwards because it was a bit faster.
        for(size_t n = terrain->grass_instances_perchunk; n > 0; n--) {

            float* xptr = &data[grasspos_index].position[0];
            float* zptr = &data[grasspos_index].position[2];
            *xptr = RSEEDRANDOMF(chunk->area.x_min, chunk->area.x_max);
            *zptr = RSEEDRANDOMF(chunk->area.z_min, chunk->area.z_max);

            RayCollision tray = raycast_terrain(terrain, *xptr, *zptr);
            data[grasspos_index].position[1] = tray.point.y;

            grasspos_index++;
        }
    }

    glBufferSubData(
            GL_SHADER_STORAGE_BUFFER,
            0,
            data_size,
            data
            );

    free(data);

    gst->init_flags |= INITFLG_GRASSDATA;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    printf("'%s': Done %li, In %0.2f seconds\n", __func__, grasspos_index, GetTime() - time_start);
}


void generate_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        u32    terrain_size,
        float  terrain_scaling,
        float  amplitude,
        float  frequency,
        int    octaves,
        int    seed
) {

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        terrain->foliage_rdata[i].matrices = NULL;
    }

    // Start by creating a height map for the terrain.

    terrain->heightmap.total_size = (terrain_size) * (terrain_size);
    terrain->heightmap.data = malloc(terrain->heightmap.total_size * sizeof *terrain->heightmap.data);
    terrain->heightmap.size = terrain_size;
    terrain->chunks = NULL;
    terrain->seed = seed;

    terrain->highest_point = 0.0;
    terrain->lowest_point = 999999.9;
    terrain->num_visible_chunks = 0;

    terrain->water_ylevel = WATER_INITIAL_YLEVEL;

    // The 'randomf' function modifies the seed.
    int seed_x = randomgen(&seed);
    int seed_z = randomgen(&seed);
 
    // First pass.
    // Base where to start crafting the heightmap from.
    render_loading_info(gst, "Generating heightmaps", 1, 4);

    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * (terrain->heightmap.size) + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * frequency;
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * frequency;

            float value = fbm_2D(p_nx, p_nz, octaves) * amplitude;
            terrain->heightmap.data[index] = (value);
        }
    }
    

    // Second pass.
    // Just for some detail.
    render_loading_info(gst, "Generating heightmaps", 2, 4);

    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/7.25);
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/7.25);

            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude * 10);
            terrain->heightmap.data[index] += value;
        }
    }

    
    // Third pass.
    // Add very low frequency noise but high amplitude. It will give an effect that its like a mountain range
    render_loading_info(gst, "Generating heightmaps", 3, 4);

    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/60.25);
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/60.25);
            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude * 80);

            terrain->heightmap.data[index] += value;
        }
    }
    
    
    // Fourth pass.
    // Some final touches. Raise the terrain at some points higher.
    // 'value':  medium amplitude noise.
    // 'value2': how much medium amplitude regions "repeat".
    // 'value3': how big the medium aplitude regions are?
    render_loading_info(gst, "Generating heightmaps", 4, 4);

    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency);
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency);
            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude*3.0);


            float p_nx2 = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/20.0);
            float p_nz2 = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/20.0);
            float value2 = fbm_2D(p_nx2, p_nz2, octaves);

            float p_nx3 = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/3.0);
            float p_nz3 = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/3.0);
            float value3 = fbm_2D(p_nx3, p_nz3, octaves);


            terrain->heightmap.data[index] += (value * (value2 * value3));
        }
    }
    

    // Get highest and lowest point.
    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float v = terrain->heightmap.data[index];
            if(v > terrain->highest_point) {
                terrain->highest_point = v;
            }

            if(v < terrain->lowest_point) {
                terrain->lowest_point = v;
            }
        }
    }


    // Raise the heighmap so 0 is at the very bottom.
    const size_t heightmap_total_size = (terrain->heightmap.size * terrain->heightmap.size);
    for(size_t i = 0; i < heightmap_total_size; i++) {
        terrain->heightmap.data[i] += fabs(terrain->lowest_point);
    }

    terrain->highest_point += fabs(terrain->lowest_point);
    terrain->lowest_point = 0.0;

    setup_biome_ylevels(gst);


    printf("Terrain (highest) point: %0.2f\n", terrain->highest_point);
    printf("Terrain (lowest)  point: %0.2f\n", terrain->lowest_point);

    SetShaderValueV(gst->shaders[DEFAULT_SHADER],
            GetShaderLocation(gst->shaders[DEFAULT_SHADER], "terrain_lowest_point"),
            &terrain->lowest_point, 1, SHADER_UNIFORM_FLOAT);

    
    size_t triangle_count = (terrain->heightmap.size-1) * (terrain->heightmap.size-1) * 2;
    size_t vertex_count = triangle_count * 3;
    
   
    // Create triangle lookup table for terrain.

    terrain->triangle_lookup = malloc(vertex_count * sizeof *terrain->triangle_lookup);
     
    Vector3 scale = (Vector3) { terrain_scaling, 1.0, terrain_scaling };
    
    for(u32 z = 0; z < terrain->heightmap.size-1; z++) {
        for(u32 x = 0; x < terrain->heightmap.size-1; x++) {
            size_t tr_index = (z * terrain->heightmap.size) + x;
            terrain->triangle_lookup[tr_index] = (struct triangle2x_t) {
            
                .a0 = (Vector3) { // 0, 1, 2
                    (float)x * scale.x,
                    get_heightmap_value(terrain, x, z),
                    (float)z * scale.z
                },
                .a1 = (Vector3) { // 3, 4, 5
                    (float)x * scale.x,
                    get_heightmap_value(terrain, x, z+1),
                    (float)(z+1) * scale.z
                },
                .a2 = (Vector3) { // 6, 7, 8
                    (float)(x+1) * scale.x,
                    get_heightmap_value(terrain, x+1, z),
                    (float)z * scale.z
                },


                .b0 = (Vector3) { // 9, 10, 11 (copy of 'a2')
                    (float)(x+1) * scale.x,
                    get_heightmap_value(terrain, x+1, z),
                    (float)z * scale.z
                },
                .b1 = (Vector3) { // 12, 13, 14 (copy of 'a1')
                    (float)x * scale.x,
                    get_heightmap_value(terrain, x, z+1),
                    (float)(z+1) * scale.z
                },
                .b2 = (Vector3) { // 15, 16, 17
                    (float)(x+1) * scale.x,
                    get_heightmap_value(terrain, x+1, z+1),
                    (float)(z+1) * scale.z
                }
            };
        }
    }
    

    terrain->scaling = terrain_scaling;

    float terrain_pos = -(terrain_size * terrain_scaling) / 2;
    terrain->transform = MatrixTranslate(terrain_pos, 0, terrain_pos);
    terrain->material = LoadMaterialDefault();
    terrain->material.shader = gst->shaders[DEFAULT_SHADER];
    terrain->material.maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[TERRAIN_TEXID];

    Vector2 terrain_origin = (Vector2){
        terrain->transform.m12,
        terrain->transform.m14
    };
    shader_setu_vec2(gst, CHUNK_FORCETEX_SHADER, U_TERRAIN_ORIGIN, &terrain_origin);

    float terrain_size_F = (float)terrain_size;
    shader_setu_float(gst, CHUNK_FORCETEX_SHADER, U_TERRAIN_SIZE, &terrain_size_F);


    // (NOT CURRENTLY USED)
    // Create plane for water
    //const float waterplane_size = (CHUNK_SIZE * (1+terrain->num_max_visible_chunks/4)) * terrain->scaling;
    terrain->waterplane = LoadModelFromMesh(GenMeshPlane(1.0, 1.0, 1, 1));
    terrain->waterplane.materials[0] = LoadMaterialDefault();
    terrain->waterplane.materials[0].shader = gst->shaders[WATER_SHADER];

 
    load_foliage_models(gst, terrain); // See 'chunk.c'

    //_load_terrain_foliage_models(gst, terrain);
    _load_terrain_chunks(gst, terrain);
    set_render_dist(gst, 3000.0); // TODO: Remove this from here.
    
   
    printf("Max visible chunks: %i\n", terrain->num_max_visible_chunks);
    printf("\033[32m -> Generated terrain succesfully.\033[0m\n");
}


void delete_terrain(struct terrain_t* terrain) {
    if(terrain->chunks) {
        for(size_t i = 0; i < terrain->num_chunks; i++) {

            delete_chunk(&terrain->chunks[i]);
        }

        free(terrain->chunks);
        terrain->chunks = NULL;
    }

    if(terrain->triangle_lookup) {
        free(terrain->triangle_lookup);
        terrain->triangle_lookup = NULL;
    }
    if(terrain->heightmap.data) {
        free(terrain->heightmap.data);
        terrain->heightmap.data = NULL;
    }


    UnloadModel(terrain->waterplane);
    
    // Free foliage data from memory.
    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        UnloadModel(terrain->foliage_models[i]);

        struct foliage_rdata_t* f_rdata = &terrain->foliage_rdata[i];
        if(f_rdata->matrices) {
            free(f_rdata->matrices);
            f_rdata->matrices = NULL;
        }
    }
    
    printf("\033[35m -> Deleted Terrain.\033[0m\n");
}




struct grass_group_t {
    int base_index;
    int num_instances;

    int num_chunks;
    float dst2player;
};


/*
// Render group of grass chunks. High quality.
static void render_grass_group(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct grass_group_t* group,
        Matrix* mvp,
        int render_pass
){


    const int mesh_triangles = terrain->grass_model.meshes[0].triangleCount;
    const int vao_id = terrain->grass_model.meshes[0].vaoId;

    shader_setu_int(gst,
            GRASSDATA_COMPUTE_SHADER,
            U_CHUNK_GRASS_BASEINDEX,
            &group->base_index
            );

    // Dispatch grassdata compute shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[GRASSDATA_SSBO]);
    dispatch_compute(gst,
            GRASSDATA_COMPUTE_SHADER,
            group->num_instances,  1, 1,
            GL_SHADER_STORAGE_BARRIER_BIT
            );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    int rshader_i = (render_pass == RENDERPASS_RESULT)
        ? TERRAIN_GRASS_SHADER
        : TERRAIN_GRASS_GBUFFER_SHADER;

    shader_setu_matrix(gst, rshader_i, U_VIEWPROJ, *mvp);

    rlEnableShader(gst->shaders[rshader_i].id);
    rlEnableVertexArray(vao_id);

    shader_setu_int(gst,
            rshader_i,
            U_CHUNK_GRASS_BASEINDEX,
            &group->base_index
            );

    rlDisableBackfaceCulling();
    glDrawElementsInstanced(
            GL_TRIANGLES,
            mesh_triangles * 3,
            GL_UNSIGNED_SHORT,
            0,
            group->num_instances
            );

    rlEnableBackfaceCulling();
    rlDisableVertexArray();
    rlDisableShader();

    terrain->num_rendered_grass += group->num_instances;
}
*/

/*
static void render_chunk_grass(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        Matrix* mvp,
        int render_pass
){

    int lowres = (chunk->dst2player > (terrain->chunk_size * terrain->scaling));

    const int mesh_triangle_count =
        lowres ? terrain->grass_model_lowres.meshes[0].triangleCount
               : terrain->grass_model.meshes[0].triangleCount;

    const int baseindex = (int)chunk->grass_baseindex;
    const int vao_id = 
        lowres ? terrain->grass_model_lowres.meshes[0].vaoId
               : terrain->grass_model.meshes[0].vaoId;

    int instances = terrain->grass_instances_perchunk;

    if(lowres) {
        instances /= 2.0;
    }
    

    shader_setu_int(gst, 
            GRASSDATA_COMPUTE_SHADER,
            U_CHUNK_GRASS_BASEINDEX,
            &baseindex
            );

    // Dispatch grassdata compute shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[GRASSDATA_SSBO]);
    dispatch_compute(gst,
            GRASSDATA_COMPUTE_SHADER,
            instances,  1, 1,
            GL_SHADER_STORAGE_BARRIER_BIT
            );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



    int rshader_i = (render_pass == RENDERPASS_RESULT)
        ? TERRAIN_GRASS_SHADER
        : TERRAIN_GRASS_GBUFFER_SHADER;


    shader_setu_matrix(gst, rshader_i, U_VIEWPROJ, *mvp);

    rlEnableShader(gst->shaders[rshader_i].id);
    rlEnableVertexArray(vao_id);
    
    shader_setu_int(gst,
            rshader_i,
            U_CHUNK_GRASS_BASEINDEX,
            &baseindex
            );

    rlDisableBackfaceCulling();
    glDrawElementsInstanced(
            GL_TRIANGLES,
            mesh_triangle_count * 3,
            GL_UNSIGNED_SHORT,
            0,
            instances
            );

    rlEnableBackfaceCulling();
    rlDisableVertexArray();
    rlDisableShader();

    terrain->num_rendered_grass += instances;
}
*/

void render_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        int render_pass
){
    const float terrain_render_timestart = GetTime();

    terrain->num_visible_chunks = 0;
    // Clear foliage render data from previous frame.

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
    struct foliage_rdata_t* f_rdata = &terrain->foliage_rdata[i];
        memset(f_rdata->matrices, 0, f_rdata->matrices_size * sizeof *f_rdata->matrices);
        f_rdata->next_index = 0;
        f_rdata->num_render = 0;
    }

    //terrain->water_ylevel += sin(gst->time)*0.001585;
    float trender_dist = 0;
    //int render_grass = 1;


    // Reduce render distance for ssao and shadow cams.
    switch(render_pass) {
        case RENDERPASS_RESULT:
            trender_dist = gst->render_dist;
            break;

        case RENDERPASS_GBUFFER: // SSAO
            trender_dist = (terrain->scaling * (terrain->chunk_size));
            break;

        case RENDERPASS_SHADOWS:
            trender_dist = (terrain->scaling * (terrain->chunk_size));
            //render_grass = 0;
            break;
    }

    //render_grass *= (gst->grass_enabled);


    int ground_pass = 1;
    shader_setu_int(gst, DEFAULT_SHADER, U_GROUND_PASS, &ground_pass);


   
    terrain->num_grass_chunks = 0;


    /*
    struct chunk_t* prev_chunk = &terrain->chunks[0];
    struct grass_group_t grass_group = { -1, 0, 0, 0 };
    */


    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        chunk->dst2player = 
            Vector3Distance(
                    (Vector3){
                        chunk->center_pos.x, 0, chunk->center_pos.z
                    },
                    (Vector3){
                        gst->player.position.x, 0, gst->player.position.z
                    });


        if(chunk->dst2player > trender_dist) {
            continue;
        }


        // Chunks very nearby may get discarded if the center position goes behind the player.
        // dont test it if its very close.
        int skip_view_test = (chunk->dst2player < (terrain->chunk_size * terrain->scaling));
        if(!skip_view_test && !point_in_player_view(gst, &gst->player, chunk->center_pos, 70.0)) {
            continue;
            //goto skip_chunk_ground;
        }

        terrain->num_visible_chunks++;
        if(terrain->num_visible_chunks >= terrain->num_max_visible_chunks) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Overloading render data array."
                    " Not rendering more this frame.\033[0m\n",
                    __func__);
            return;
        }


        // Copy current foliage type matrices from chunk to bigger array of same type.

        for(size_t k = 0; k < MAX_FOLIAGE_TYPES; k++) {
            struct foliage_rdata_t*       f_rdata     = &terrain->foliage_rdata[k];
            struct chunk_foliage_data_t*  chunk_fdata = &chunk->foliage_data[k];


            if(chunk->foliage_data[k].matrices == NULL) {
                fprintf(stderr, "\033[31m(ERROR) '%s': chunk foliage data is NULL\033[0m\n",
                        __func__);
                continue;
            }
         

            memmove(
                    &f_rdata->matrices[f_rdata->next_index],
                    chunk_fdata->matrices,
                    chunk_fdata->num_foliage * sizeof(Matrix)
                    );

            f_rdata->next_index += chunk_fdata->num_foliage;
            f_rdata->num_render += chunk_fdata->num_foliage;

        }

        // Render chunk ground.
        Matrix translation = MatrixTranslate(chunk->position.x, 0, chunk->position.z);
        DrawMesh(terrain->chunks[i].mesh, terrain->biome_materials[chunk->biome.id], translation);
        

        if(gst->grass_enabled) {
            //render_chunk_grass(gst, terrain, chunk, &mvp, render_pass);

            if(gst->terrain.num_grass_chunks < MAX_GRASS_CHUNKS) {
                gst->terrain.grass_chunks[gst->terrain.num_grass_chunks] = chunk;
                gst->terrain.num_grass_chunks++;
            }

            //write_chunk_forcetex(gst, chunk);
            //render_chunk_grass(gst, terrain, chunk, &mvp, render_pass);
            
            /*
            if(chunk->dst2player > (terrain->scaling * terrain->chunk_size)) {
                // Render individual chunks grass but very low quality
                // And it reduces the number of instances.
                render_chunk_grass_lowres(gst, terrain, chunk, &mvp, render_pass);
                continue;
            }


            grass_group.num_chunks++;

            if(!FloatEquals(prev_chunk->center_pos.z, chunk->center_pos.z)) {
                grass_group.num_instances = (grass_group.num_chunks * grass_perchunk);

                // Render high quality grass blade.
                render_grass_group(gst, terrain, &grass_group, &mvp, render_pass);

                grass_group = (struct grass_group_t) { -1, 0, 0, 0 };
            }
           
            if(grass_group.base_index < 0) {
                grass_group.base_index = chunk->grass_baseindex;
                grass_group.dst2player = chunk->dst2player;
            }
            
            prev_chunk = chunk;
            */
        }
    }

    /*
    if(render_grass) {

        // Finish last grass group if valid.
        if(grass_group.base_index >= 0) {
            grass_group.num_instances = ((1+grass_group.num_chunks) * grass_perchunk);
            render_grass_group(gst, terrain, &grass_group, &mvp, render_pass);
        }
    }
    */

    ground_pass = 0;
    shader_setu_int(gst, DEFAULT_SHADER, U_GROUND_PASS, &ground_pass);
    

    // Render foliage.

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        Model* fmodel = &terrain->foliage_models[i];

        if(fmodel->materialCount < fmodel->meshCount) {
            fprintf(stderr, "\033[35m(WARNING) '%s': foliage model(type:%li) mesh count doesnt match material count??\n",
                    __func__, i);
        }

        for(int mi = 0; mi < fmodel->meshCount; mi++) {
            size_t mat_index = (size_t)CLAMP(mi, 0, fmodel->materialCount);

            if(terrain->foliage_rdata[i].render_backface) {
                rlDisableBackfaceCulling();
            }

            DrawMeshInstanced(
                    fmodel->meshes[mi],
                    fmodel->materials[mat_index],
                    terrain->foliage_rdata[i].matrices,
                    terrain->foliage_rdata[i].num_render
                    );
                
            rlEnableBackfaceCulling();
        }
    }

    state_timebuf_add(gst, 
            TIMEBUF_ELEM_TERRAIN_R,
            GetTime() - terrain_render_timestart);

}

