#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "terrain.h"
#include "state/state.h"

#include "util.h"

#include <rlgl.h>


// -----------------------------------------------------------
// NOTE: X and Z must be set accordingly with terrain X,Z positions and scaling.
// -----------------------------------------------------------
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


static void _load_foliage_model(struct state_t* gst, Model* modelptr, const char* model_filepath) {
    *modelptr = LoadModel(model_filepath);
    
    for(int i = 0; i < modelptr->materialCount; i++) {
        modelptr->materials[i] = LoadMaterialDefault();
        modelptr->materials[i].shader = gst->shaders[FOLIAGE_SHADER];
    }
}

static void _load_terrain_foliage_models(struct state_t* gst, struct terrain_t* terrain) {
   

    _load_foliage_model(gst, &terrain->foliage_models[TF_TREE_TYPE0], "res/models/tree_type0.glb");
    terrain->foliage_models[TF_TREE_TYPE0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
        = gst->textures[TREEBARK_TEXID];
    terrain->foliage_models[TF_TREE_TYPE0].materials[1].maps[MATERIAL_MAP_DIFFUSE].texture
        = gst->textures[LEAF_TEXID];

    terrain->foliage_max_perchunk[TF_TREE_TYPE0] = 64;



    _load_foliage_model(gst, &terrain->foliage_models[TF_TREE_TYPE1], "res/models/tree_type1.glb");
    terrain->foliage_models[TF_TREE_TYPE1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
        = gst->textures[TREEBARK_TEXID];
    terrain->foliage_models[TF_TREE_TYPE1].materials[1].maps[MATERIAL_MAP_DIFFUSE].texture
        = gst->textures[LEAF_TEXID];
    
    terrain->foliage_max_perchunk[TF_TREE_TYPE1] = 32;



    _load_foliage_model(gst, &terrain->foliage_models[TF_ROCK_TYPE0], "res/models/rock_type0.glb");
    terrain->foliage_models[TF_ROCK_TYPE0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
        = gst->textures[ROCK_TEXID];

    terrain->foliage_max_perchunk[TF_ROCK_TYPE0] = 16;


    _load_foliage_model(gst, &terrain->foliage_models[TF_MUSHROOM_TYPE0], "res/models/mushroom.glb");
    terrain->foliage_models[TF_MUSHROOM_TYPE0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture
        = gst->textures[TERRAIN_MUSHROOM_TEXID];
    terrain->foliage_max_perchunk[TF_MUSHROOM_TYPE0] = 50;

}

#define CLEAR_BACKGROUND ClearBackground((Color){ 10, 10, 10, 255 })


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

// RENAME THIS FUNCTIONS:
static void _load_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        size_t max_perchunk = terrain->foliage_max_perchunk[i];
        
        if(max_perchunk == 0) {
            fprintf(stderr, "\033[35m(WARNING) '%s': Foliage type %li, max perchunk is zero!\033[0m\n",
                    __func__, i);
            continue;
        }

        chunk->foliage_data[i].matrices = malloc(max_perchunk * sizeof(Matrix));
        chunk->foliage_data[i].matrices_size = max_perchunk;
        chunk->foliage_data[i].num_foliage = 0;
    }

    const float x_min = chunk->position.x;
    const float x_max = chunk->position.x + (terrain->chunk_size * terrain->scaling);
    const float z_min = chunk->position.z;
    const float z_max = chunk->position.z + (terrain->chunk_size * terrain->scaling);


    struct chunk_foliage_data_t* chunk_fdata = NULL;
    

    // --- TREE TYPE 0
    chunk_fdata = &chunk->foliage_data[TF_TREE_TYPE0];
    for(size_t i = 0; i < chunk_fdata->matrices_size; i++) {
    
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);
        /*
        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }
        */

        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation    = MatrixRotateY(RSEEDRANDOMF(-M_PI, M_PI));

        chunk_fdata->matrices[chunk_fdata->num_foliage] = MatrixMultiply(rotation, translation);
        chunk_fdata->num_foliage++;
    }


    // --- TREE TYPE 1
    chunk_fdata = &chunk->foliage_data[TF_TREE_TYPE1];
    for(size_t i = 0; i < chunk_fdata->matrices_size; i++) {
    
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);
        /*
        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }
        */


        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation    = MatrixRotateY(RSEEDRANDOMF(-M_PI, M_PI));

        chunk_fdata->matrices[chunk_fdata->num_foliage] = MatrixMultiply(rotation, translation);
        chunk_fdata->num_foliage++;
    }

    // --- ROCK TYPE 0
    chunk_fdata = &chunk->foliage_data[TF_ROCK_TYPE0];
    for(size_t i = 0; i < chunk_fdata->matrices_size; i++) {
    
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);
        /*
        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }
        */

        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation    = MatrixRotateXYZ(
                                (Vector3){ RSEEDRANDOMF(-M_PI, M_PI), RSEEDRANDOMF(-M_PI, M_PI), RSEEDRANDOMF(-M_PI, M_PI) });

        chunk_fdata->matrices[chunk_fdata->num_foliage] = MatrixMultiply(rotation, translation);
        chunk_fdata->num_foliage++;
    }


    // --- MUSHROOM TYPE 0
    chunk_fdata = &chunk->foliage_data[TF_MUSHROOM_TYPE0];
    for(size_t i = 0; i < chunk_fdata->matrices_size; i++) {
   
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);
       
        float noise = perlin_noise_2D(x*0.00025, z*0.00025);
        
        if(noise < 0.0) {
            continue;
        }

        RayCollision ray = raycast_terrain(terrain, x, z);
        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }


        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation    = MatrixRotateY(RSEEDRANDOMF(-M_PI, M_PI));
        chunk_fdata->matrices[chunk_fdata->num_foliage] = MatrixMultiply(rotation, translation);
        chunk_fdata->num_foliage++;
    }
}

static void _decide_chunk_biome(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

    chunk->biome = terrain->biomedata[BIOMEID_COMFY];
    /*
    long int chunks_inrow = (terrain->heightmap.size / terrain->chunk_size);
    const float chunks_inrow_F = (float)chunks_inrow;

    if(chunks_inrow <= 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'chunks_inrow' is zero or less\033[0m\n",
                __func__);
        return;
    }

    long int chunk_x = chunk->index % chunks_inrow;
    long int chunk_z = chunk->index / chunks_inrow;

    chunk_x += terrain->seed;
    chunk_z += terrain->seed;

    float freq = 4.0;

    float pnoise = perlin_noise_2D(
            freq * ((float)chunk_x / chunks_inrow_F),
            freq * ((float)chunk_z / chunks_inrow_F));

    pnoise *= 255.0;
    pnoise /= 80.0;
    pnoise = CLAMP(pnoise, -1.0, 1.0);


    float t = (round((pnoise * MAX_BIOME_TYPES + MAX_BIOME_TYPES) / MAX_BIOME_TYPES));

    // Just make sure it is in the valid range.
    t = CLAMP(t, 0, MAX_BIOME_TYPES);
    int biome_id = (int)t;

    chunk->biome = terrain->biomedata[biome_id];
    */
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
    // Then they can be rendered individually.

    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        *chunk = (struct chunk_t) { 0 };
        chunk->index = i;
   
        
        // Loading bar
        BeginDrawing();
        {
            CLEAR_BACKGROUND;

            DrawText(TextFormat("Loading chunk (%i / %i)", i, terrain->num_chunks), 100, 100, 40.0, WHITE);
            const float bar_x = 100;
            const float bar_y = 200;
            const float bar_max_x = 800;
            float bar_height = 20;
            float bar_width = map((float)i, 0, (float)terrain->num_chunks, 0.0, bar_max_x);
            DrawRectangle(bar_x-5, bar_y-5, bar_max_x+10, bar_height+10, (Color){ 30, 40, 30, 255 });
            DrawRectangle(bar_x, bar_y, bar_width, bar_height, (Color){ 80, 200, 80, 255 });

        }
        EndDrawing();

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
   
        _decide_chunk_biome(gst, terrain, chunk);
        _load_chunk_foliage(gst, terrain, chunk);

        UploadMesh(&chunk->mesh, 0);

        chunk_x += terrain->chunk_size;
        if(chunk_x >= terrain_size) {
            chunk_x = 0;
            chunk_z += terrain->chunk_size;
        }
    }

    printf("\n----------------\n");
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
    
    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 1 / 4", 100, 100, 40.0, WHITE);
    EndDrawing();   

    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * (terrain->heightmap.size) + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * frequency;
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * frequency;

            float value = fbm_2D(p_nx, p_nz, octaves) * amplitude;
            terrain->heightmap.data[index] = value;
        }
    }
    

    // Second pass.

    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 2 / 4", 100, 100, 40.0, WHITE);
    EndDrawing();   


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

    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 3 / 4", 100, 100, 40.0, WHITE);
    EndDrawing();   


    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/50.25);
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/50.25);

            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude * 100);
            terrain->heightmap.data[index] += value;
        }
    }
    
    
    // Fourth pass.

    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 4 / 4", 100, 100, 40.0, WHITE);
    EndDrawing();


    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency);
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency);
            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude*3.0);


            float p_nx2 = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/10.0);
            float p_nz2 = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/10.0);
            float value2 = fbm_2D(p_nx2, p_nz2, octaves);

            float p_nx3 = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/2.0);
            float p_nz3 = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/2.0);
            float value3 = fbm_2D(p_nx3, p_nz3, octaves);


            terrain->heightmap.data[index] += value * (value2 * value3);
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

    printf("Terrain highest point: %0.2f\n", terrain->highest_point);
    printf("Terrain lowest point: %0.2f\n", terrain->lowest_point);

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

 


    
    _load_terrain_foliage_models(gst, terrain);
    _load_terrain_chunks(gst, terrain);
     set_render_dist(gst, 3000.0);
   

    // Get valid spawnpoint for player.
    {
        int attemps = 0;
        int max_attemps = 100;
        const float spawn_rad = (terrain_size * terrain_scaling) / 4.0;
        while(1) {
            terrain->valid_player_spawnpoint = (Vector3) {
                RSEEDRANDOMF(-spawn_rad, spawn_rad),
                0,
                RSEEDRANDOMF(-spawn_rad, spawn_rad)
            };

            RayCollision ray = raycast_terrain(terrain,
                    terrain->valid_player_spawnpoint.x,
                    terrain->valid_player_spawnpoint.z
                    );

            if(ray.point.y > terrain->water_ylevel) { 
                break;
            }

            attemps++;
            if(attemps >= max_attemps) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Cant find valid spawn point.\033[0m\n",
                        __func__);
                break;
            }
        }
    }


    printf("Max visible chunks: %i\n", terrain->num_max_visible_chunks);
    printf("\033[32m -> Generated terrain succesfully.\033[0m\n");
}


void delete_terrain(struct terrain_t* terrain) {
    if(terrain->chunks) {
        for(size_t i = 0; i < terrain->num_chunks; i++) {
            for(int j = 0; j < MAX_FOLIAGE_TYPES; j++) {
                free(terrain->chunks[i].foliage_data[j].matrices);
            }

            UnloadMesh(terrain->chunks[i].mesh);
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


void render_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
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
    float trender_dist = (render_setting == RENDER_TERRAIN_FOR_PLAYER) ? gst->render_dist : 1200.0;

    int ground_pass = 1;
    shader_setu_int(gst, DEFAULT_SHADER, U_GROUND_PASS, &ground_pass);

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

        if(!skip_view_test && !point_in_player_view(gst, &gst->player, chunk->center_pos, 100.0)) {
            continue;
        }

        terrain->num_visible_chunks++;


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

        Matrix translation = MatrixTranslate(chunk->position.x, 0, chunk->position.z);
        DrawMesh(terrain->chunks[i].mesh, terrain->biome_materials[chunk->biome.id], translation);
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

            DrawMeshInstanced(
                    fmodel->meshes[mi],
                    fmodel->materials[mat_index],
                    terrain->foliage_rdata[i].matrices,
                    terrain->foliage_rdata[i].num_render
                    );
        }
    }
}


