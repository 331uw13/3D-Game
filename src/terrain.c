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

static float get_heightmap_value(struct terrain_t* terrain, float x, float z) {
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
    
    printf("--------- LOADING CHUNKS ---------\n\n");

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

    
    Vector3 scale = (Vector3) { terrain->scaling, 1.0, terrain->scaling };

    // Allocate memory for chunks and fill them with terrain vertices and normals.

    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        *chunk = (struct chunk_t) { 0 };
        chunk->index = i;
  

        render_loading_info(gst, "Loading chunks",
                i, terrain->num_chunks);

        chunk->mesh.triangleCount = chunk_triangle_count;
        chunk->mesh.vertexCount = chunk->mesh.triangleCount * 3;

        chunk->mesh.vertices = malloc(chunk->mesh.vertexCount * 3 * sizeof(float));
        chunk->mesh.normals  = malloc(chunk->mesh.vertexCount * 3 * sizeof(float));
        /*TODO*/chunk->mesh.texcoords = malloc(chunk->mesh.vertexCount * 2 * sizeof(float));

        // Used for calculating normals.
        Vector3 vA = { 0 };
        Vector3 vB = { 0 };
        Vector3 vC = { 0 };

        int v_counter = 0; // Count vertices.
        int n_counter = 0; // Count normals.
        int tc_counter = 0; // Count texcoords.

        // Figure chunk Y position by taking avarage of all chunk y vertices.
        // (Chunk Y is not used in rendering because the y level is just from the heightmap)
        chunk->position = (Vector3){ 
            (chunk_x * (terrain->scaling)) - ((terrain_size/2) * terrain->scaling),
            0, 
            (chunk_z * (terrain->scaling)) - ((terrain_size/2) * terrain->scaling)
        };

        chunk->center_pos = (Vector3) {
            chunk->position.x
                + (terrain->scaling * terrain->chunk_size / 2),
            0,
            chunk->position.z
                + (terrain->scaling * terrain->chunk_size / 2)
        };

        // Used to get average Y position of chunk.
        float vertex_y_points = 0.0;
        float num_vertex_y_points = 0.0;

        for(int z = 0; z < terrain->chunk_size; z++) {
            for(int x = 0; x < terrain->chunk_size; x++) {
       

                float X = (float)(x + chunk_x);
                float Z = (float)(z + chunk_z);

                // TODO: Remove this shit and fix the heightmap size.
                int last_x = (X+1 >= terrain_size);
                int last_z = (Z+1 >= terrain_size);
                X = CLAMP(X - last_x, 0, terrain_size);
                Z = CLAMP(Z - last_z, 0, terrain_size);

                // Left up corner triangle

                chunk->mesh.vertices[v_counter]   = (float)x * scale.x;
                chunk->mesh.vertices[v_counter+1] = get_heightmap_value(terrain, X, Z);
                chunk->mesh.vertices[v_counter+2] = (float)z * scale.z;
                
                chunk->mesh.vertices[v_counter+3] = (float)x * scale.x;
                chunk->mesh.vertices[v_counter+4] = get_heightmap_value(terrain, X, Z+1);
                chunk->mesh.vertices[v_counter+5] = (float)(z+1) * scale.z;
                
                chunk->mesh.vertices[v_counter+6] = (float)(x+1) * scale.x;
                chunk->mesh.vertices[v_counter+7] = get_heightmap_value(terrain, X+1, Z);
                chunk->mesh.vertices[v_counter+8] = (float)z * scale.z;

                // Average of the quad y points.
                // Then later take avarage of those.
                vertex_y_points += 
                    ( chunk->mesh.vertices[v_counter+1]
                    + chunk->mesh.vertices[v_counter+4]
                    + chunk->mesh.vertices[v_counter+7]) / 3.0;

                num_vertex_y_points += 1.0;

                // Right bottom corner triangle

                chunk->mesh.vertices[v_counter+9] = chunk->mesh.vertices[v_counter+6];
                chunk->mesh.vertices[v_counter+10] = chunk->mesh.vertices[v_counter+7];
                chunk->mesh.vertices[v_counter+11] = chunk->mesh.vertices[v_counter+8];
                
                chunk->mesh.vertices[v_counter+12] = chunk->mesh.vertices[v_counter+3];
                chunk->mesh.vertices[v_counter+13] = chunk->mesh.vertices[v_counter+4];
                chunk->mesh.vertices[v_counter+14] = chunk->mesh.vertices[v_counter+5];
                
                chunk->mesh.vertices[v_counter+15] = (float)(x+1) * scale.x;
                chunk->mesh.vertices[v_counter+16] = get_heightmap_value(terrain, X+1, Z+1);
                chunk->mesh.vertices[v_counter+17] = (float)(z+1) * scale.z;



                v_counter += 18;

                
                // Calulcate normals

                for(int i = 0; i < 18; i += 9) {
                    vA.x = chunk->mesh.vertices[n_counter + i];
                    vA.y = chunk->mesh.vertices[n_counter + i+1];
                    vA.z = chunk->mesh.vertices[n_counter + i+2];

                    vB.x = chunk->mesh.vertices[n_counter + i+3];
                    vB.y = chunk->mesh.vertices[n_counter + i+4];
                    vB.z = chunk->mesh.vertices[n_counter + i+5];
                    
                    vC.x = chunk->mesh.vertices[n_counter + i+6];
                    vC.y = chunk->mesh.vertices[n_counter + i+7];
                    vC.z = chunk->mesh.vertices[n_counter + i+8];

                    Vector3 vN 
                        = Vector3Normalize(
                            Vector3CrossProduct(
                                Vector3Subtract(vB, vA),
                                Vector3Subtract(vC, vA)
                                )
                            );

                    chunk->mesh.normals[n_counter + i]   = vN.x;
                    chunk->mesh.normals[n_counter + i+1] = vN.y;
                    chunk->mesh.normals[n_counter + i+2] = vN.z;
                    chunk->mesh.normals[n_counter + i+3] = vN.x;
                    chunk->mesh.normals[n_counter + i+4] = vN.y;
                    chunk->mesh.normals[n_counter + i+5] = vN.z;
                    chunk->mesh.normals[n_counter + i+6] = vN.x;
                    chunk->mesh.normals[n_counter + i+7] = vN.y;
                    chunk->mesh.normals[n_counter + i+8] = vN.z;
                }

                // Texture coordinates.
                const float m_s = (float)8 - 1;

                float tx = (float)x;
                float tz = (float)z;

                chunk->mesh.texcoords[tc_counter]   = tx / m_s;
                chunk->mesh.texcoords[tc_counter+1] = tz / m_s;
                
                chunk->mesh.texcoords[tc_counter+2] = tx / m_s;
                chunk->mesh.texcoords[tc_counter+3] = (tz+1) / m_s;
                
                chunk->mesh.texcoords[tc_counter+4] = (tx+1) / m_s;
                chunk->mesh.texcoords[tc_counter+5] = tz / m_s;
                
                chunk->mesh.texcoords[tc_counter+6] = chunk->mesh.texcoords[tc_counter + 4];
                chunk->mesh.texcoords[tc_counter+7] = chunk->mesh.texcoords[tc_counter + 5];
                
                chunk->mesh.texcoords[tc_counter+8] = chunk->mesh.texcoords[tc_counter + 2];
                chunk->mesh.texcoords[tc_counter+9] = chunk->mesh.texcoords[tc_counter + 3];
                
                chunk->mesh.texcoords[tc_counter+10] = (tx+1) / m_s;
                chunk->mesh.texcoords[tc_counter+11] = (tz+1) / m_s;

                tc_counter += 12;

                n_counter += 18;
            }
        }

        chunk->position.y = vertex_y_points / num_vertex_y_points;
        chunk->center_pos.y = chunk->position.y;

        decide_chunk_biome(gst, terrain, chunk);
        load_chunk_foliage(gst, terrain, chunk);

        UploadMesh(&chunk->mesh, 0);

        chunk_x += terrain->chunk_size;
        if(chunk_x >= terrain_size) {
            chunk_x = 0;
            chunk_z += terrain->chunk_size;
        }
    }

    printf("\n----------------\n");
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
        float _reserved[4];
    };

    const size_t data_size = 
        (terrain->num_chunks * terrain->grass_instances_perchunk)
        * (sizeof(float) * 8);

    struct grassdata_t* data = malloc(data_size);

    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        chunk->grass_baseindex = grasspos_index;

        const float chunk_x_min = chunk->position.x;
        const float chunk_x_max = chunk->position.x + (terrain->chunk_size * terrain->scaling);
        const float chunk_z_min = chunk->position.z;
        const float chunk_z_max = chunk->position.z + (terrain->chunk_size * terrain->scaling);

        render_loading_info(gst, "Loading Grass", i, terrain->num_chunks);

        // Making this loop backwards because it was a bit faster.
        for(size_t n = terrain->grass_instances_perchunk; n > 0; n--) {

            //float grasspos[4] = { 0, 0, 0, 0 };
            data[grasspos_index].position[0] = RSEEDRANDOMF(chunk_x_min, chunk_x_max);
            data[grasspos_index].position[2] = RSEEDRANDOMF(chunk_z_min, chunk_z_max);

            data[grasspos_index].position[1] 
                = raycast_terrain(terrain, 
                        data[grasspos_index].position[0],
                        data[grasspos_index].position[2]).point.y;
            
            /*
            grasspos[0] = RSEEDRANDOMF(chunk_x_min, chunk_x_max);
            grasspos[2] = RSEEDRANDOMF(chunk_z_min, chunk_z_max);
            grasspos[1] = raycast_terrain(terrain, grasspos[0], grasspos[2]).point.y;
            */
            /*
            glBufferSubData(
                    GL_SHADER_STORAGE_BUFFER,
                    grasspos_index * GRASSDATA_STRUCT_SIZE,
                    sizeof(float)*4,
                    grasspos
                    );
                    */
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
    terrain->lowest_point = 999999;
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
            for(int j = 0; j < MAX_FOLIAGE_TYPES; j++) {
                free(terrain->chunks[i].foliage_data[j].matrices);
            }

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
    
    printf("\033[35m -> Deleted Terrain\033[0m\n");
}

/*
static void render_chunk_grass(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        Matrix* mvp,
        int renderpass
){
    if(chunk->dst2player > terrain->grass_render_dist) {
        return;
    }
    const int mesh_triangle_count = terrain->grass_model.meshes[0].triangleCount;
    const int baseindex = (int)chunk->grass_baseindex;


    shader_setu_int(gst, 
            GRASSDATA_COMPUTE_SHADER,
            U_CHUNK_GRASS_BASEINDEX,
            &baseindex
            );

    // Dispatch grassdata compute shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[GRASSDATA_SSBO]);
    dispatch_compute(gst,
            GRASSDATA_COMPUTE_SHADER,
            terrain->grass_instances_perchunk,  1, 1,
            GL_SHADER_STORAGE_BARRIER_BIT
            );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



    int render_shader_i = (renderpass == RENDERPASS_RESULT)
        ? TERRAIN_GRASS_SHADER : TERRAIN_GRASS_GBUFFER_SHADER;

    shader_setu_matrix(gst, render_shader_i, U_VIEWPROJ, *mvp);

    rlEnableShader(gst->shaders[render_shader_i].id);
    rlEnableVertexArray(
            (chunk->dst2player > terrain->grass_render_dist / 2.0)
            ? terrain->grass_model_lowres.meshes[0].vaoId
            : terrain->grass_model.meshes[0].vaoId
            );
    
    shader_setu_int(gst,
            render_shader_i,
            U_CHUNK_GRASS_BASEINDEX,
            &baseindex
            );

    rlDisableBackfaceCulling();
    glDrawElementsInstanced(
            GL_TRIANGLES,
            mesh_triangle_count * 3,
            GL_UNSIGNED_SHORT,
            0,
            terrain->grass_instances_perchunk
            );

    rlEnableBackfaceCulling();
    rlDisableVertexArray();
    rlDisableShader();

    terrain->num_rendered_grass += terrain->grass_instances_perchunk;
}
*/


struct grass_group_t {
    int base_index;
    int num_instances;
    Vector3 center;
};
static void render_grass_group(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct grass_group_t* group,
        Matrix* mvp,
        int renderpass
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


    int rshader_i = (renderpass == RENDERPASS_RESULT)
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


void render_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        int renderpass,
        int render_setting
){
    terrain->num_visible_chunks = 0;
    // Clear foliage render data from previous frame.

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
    struct foliage_rdata_t* f_rdata = &terrain->foliage_rdata[i];
        memset(f_rdata->matrices, 0, f_rdata->matrices_size * sizeof *f_rdata->matrices);
        f_rdata->next_index = 0;
        f_rdata->num_render = 0;
    }

    terrain->water_ylevel += sin(gst->time)*0.001585;
    float trender_dist = (render_setting == RENDER_TERRAIN_FOR_PLAYER) 
        ? gst->render_dist : SHADOW_CAM_RENDERDIST;

    int ground_pass = 1;
    shader_setu_int(gst, DEFAULT_SHADER, U_GROUND_PASS, &ground_pass);


    Matrix mvp = MatrixMultiply(
            rlGetMatrixModelview(),
            rlGetMatrixProjection()
            );  
   
    terrain->num_rendered_grass = 0;


    // Add new chunk base index and increase instance count on the group.
    // If chunk Z Position changes Render current group and start collecting new group info.
    // This will reduce draw calls and compute dispatches.

    struct chunk_t* prev_chunk = &terrain->chunks[0];
    //struct grass_group_t grass_groups[32] = { 0 };
    //size_t grass_group_i = 0;

    struct grass_group_t grass_group = { -1, 0 };

    int render_grass = gst->grass_enabled && (((renderpass == RENDERPASS_RESULT)
        || (renderpass == RENDERPASS_GBUFFER && gst->ssao_enabled))
        && render_setting == RENDER_TERRAIN_FOR_PLAYER);

    int grass_perchunk = terrain->grass_instances_perchunk;
    int first_group_render = 1;

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
        // dont need to test it if its very close.
        int skip_view_test = (chunk->dst2player < (terrain->chunk_size * terrain->scaling));
        if(!skip_view_test && !point_in_player_view(gst, &gst->player, chunk->center_pos, 60.0)) {
            goto skip_chunk_ground;
            //continue;
        }

        terrain->num_visible_chunks++;
        if(terrain->num_visible_chunks >= terrain->num_max_visible_chunks) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Overloading render data array. Not rendering more this frame.\033[0m\n",
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

skip_chunk_ground:

        // The nearby acceptable distance must be increased a littlebit.
        // Otherwise some grass will be clipped off.
        int skip_grass_view_test = (chunk->dst2player < (terrain->chunk_size * terrain->scaling)*2);
        if(!skip_grass_view_test && !point_in_player_view(gst, &gst->player, chunk->center_pos, 60.0)) {
            continue;
        }

        if(render_grass) {
            if(grass_group.base_index < 0) {
                grass_group.base_index = chunk->grass_baseindex;
                if(!first_group_render) {
                    grass_group.base_index -= grass_perchunk;
                }
            }


            grass_group.num_instances += grass_perchunk;
           
            // If this statement is true  we can know the row has been changed
            // and current group must be rendered.
            if(!FloatEquals(prev_chunk->center_pos.z, chunk->center_pos.z)) {
                /*
                Vector3 pp = chunk->center_pos;
                pp.y += 600;
                DrawSphere(pp, 30.0, RED);
                */
                render_grass_group(gst, terrain, &grass_group, &mvp, renderpass);
                grass_group.base_index = -1;
                grass_group.num_instances = 0;
            }

            first_group_render = 0;
            prev_chunk = chunk;
        }
    }

    if(render_grass) {
        // Finish last grass group.
        grass_group.num_instances += grass_perchunk;
        render_grass_group(gst, terrain, &grass_group, &mvp, renderpass);
    }

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
}


