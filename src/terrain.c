#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "terrain.h"
#include "state.h"

#include "util.h"


// -----------------------------------------------------------
// NOTE: X and Z must be set accordingly with terrain X,Z positions and scaling.
// -----------------------------------------------------------
static size_t get_heightmap_index(struct terrain_t* terrain, float x, float z) {

    int r_x = round(x);
    int r_z = round(z);

    long int i = (r_z * terrain->heightmap.size + r_x);

    i = (i < 0) ? 0 : (i > terrain->heightmap.total_size) ? 0 : i;


    return (size_t)i;
}


static float get_heightmap_value(struct terrain_t* terrain, float x, float z) {
    size_t index = get_heightmap_index(terrain, x, z);
    return terrain->heightmap.data[index];
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

static void _load_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {
    
    struct foliage_matrices_t* fm = &chunk->foliage_matrices;

    const float x_min = chunk->position.x;
    const float x_max = chunk->position.x + (terrain->chunk_size * terrain->scaling);
    const float z_min = chunk->position.z;
    const float z_max = chunk->position.z + (terrain->chunk_size * terrain->scaling);


    // Tree Type0
    fm->num_tree_type0 = TREE_TYPE0_MAX_PERCHUNK;
    for(size_t i = 0; i < fm->num_tree_type0; i++) {
        
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);

        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }

        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation = MatrixRotateY(RSEEDRANDOMF(0.0, 360.0)*DEG2RAD);
        fm->tree_type0[i] = MatrixMultiply(rotation, translation);
    }

    // Tree Type1
    fm->num_tree_type1 = TREE_TYPE1_MAX_PERCHUNK;
    for(size_t i = 0; i < fm->num_tree_type1; i++) {
        
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);

        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }

        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation = MatrixRotateY(RSEEDRANDOMF(0.0, 360.0)*DEG2RAD);
        fm->tree_type1[i] = MatrixMultiply(rotation, translation);
    }

    // Rock Type0
    fm->num_rock_type0 = ROCK_TYPE0_MAX_PERCHUNK;
    for(size_t i = 0; i < fm->num_rock_type0; i++) {
        
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);

        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation = MatrixRotateXYZ(
                (Vector3){
                    RSEEDRANDOMF(0, 360) * DEG2RAD,
                    RSEEDRANDOMF(0, 360) * DEG2RAD,
                    RSEEDRANDOMF(0, 360) * DEG2RAD
                });
        fm->rock_type0[i] = MatrixMultiply(rotation, translation);
    }

    // Crystals
    fm->num_crystals = CRYSTALS_MAX_PERCHUNK;
    for(size_t i = 0; i < fm->num_crystals; i++) {
        
        float x = RSEEDRANDOMF(x_min, x_max);
        float z = RSEEDRANDOMF(z_min, z_max);

        RayCollision ray = raycast_terrain(terrain, x, z);
        
        if(ray.point.y < terrain->water_ylevel) {
            continue;
        }

        Matrix translation = MatrixTranslate(x, ray.point.y, z);
        Matrix rotation = MatrixRotateXYZ(
                (Vector3){
                    RSEEDRANDOMF(0, 360) * DEG2RAD,
                    RSEEDRANDOMF(0, 360) * DEG2RAD,
                    RSEEDRANDOMF(0, 360) * DEG2RAD
                });
        fm->crystals[i] = MatrixMultiply(rotation, translation);
    }

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
    // Start by creating a height map for the terrain.



    terrain->heightmap.total_size = (terrain_size) * (terrain_size);
    terrain->heightmap.data = malloc(terrain->heightmap.total_size * sizeof *terrain->heightmap.data);
    terrain->heightmap.size = terrain_size;
    terrain->chunks = NULL;

    terrain->highest_point = 0.0;
    terrain->lowest_point = 999999;
    terrain->num_visible_chunks = 0;

    terrain->water_ylevel = WATER_INITIAL_YLEVEL;

    // The 'randomf' function modifies the seed.
    int seed_x = randomgen(&seed);
    int seed_z = randomgen(&seed);
 
    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 1 / 3", 100, 100, 40.0, WHITE);
    EndDrawing();   

    // First pass.
    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * (terrain->heightmap.size) + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * frequency;
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * frequency;

            float value = fbm_2D(p_nx, p_nz, octaves) * amplitude;
            terrain->heightmap.data[index] = value;
        }
    }

    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 2 / 3", 100, 100, 40.0, WHITE);
    EndDrawing();   


    // Second pass.
    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = z * terrain->heightmap.size + x;

            float p_nx = ((float)(x+seed_x) / (float)terrain->heightmap.size) * (frequency/7.25);
            float p_nz = ((float)(z+seed_z) / (float)terrain->heightmap.size) * (frequency/7.25);

            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude * 10);
            terrain->heightmap.data[index] += value;
        }
    }

    BeginDrawing();
    CLEAR_BACKGROUND;
    DrawText("Creating heightmap 3 / 3", 100, 100, 40.0, WHITE);
    EndDrawing();   


    // Third pass.
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
    terrain->material.maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[MOSS_TEXID];


    // Load foliage models
    {
        struct foliage_models_t* fmodels = &terrain->foliage_models;

        // Tree Type 0
        fmodels->tree_type0 = LoadModel("res/models/tree_type0.glb");
        
        // Tree bark
        fmodels->tree_type0.materials[0] = LoadMaterialDefault();
        fmodels->tree_type0.materials[0].shader = gst->shaders[FOLIAGE_SHADER];
        fmodels->tree_type0.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture 
            = gst->textures[TREEBARK_TEXID];
        
        // Tree leafs
        fmodels->tree_type0.materials[1] = LoadMaterialDefault();
        fmodels->tree_type0.materials[1].shader = gst->shaders[FOLIAGE_SHADER];
        fmodels->tree_type0.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture 
            = gst->textures[LEAF_TEXID];



        // Tree Type 1
        fmodels->tree_type1 = LoadModel("res/models/tree_type1.glb");
        
        // Tree bark
        fmodels->tree_type1.materials[0] = LoadMaterialDefault();
        fmodels->tree_type1.materials[0].shader = gst->shaders[FOLIAGE_SHADER];
        fmodels->tree_type1.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture 
            = gst->textures[TREEBARK_TEXID];
        
        // Tree leafs
        fmodels->tree_type1.materials[1] = LoadMaterialDefault();
        fmodels->tree_type1.materials[1].shader = gst->shaders[FOLIAGE_SHADER];
        fmodels->tree_type1.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture 
            = gst->textures[LEAF_TEXID];



        // Rock type 0
        fmodels->rock_type0 = LoadModel("res/models/rock_type0.glb");
        fmodels->rock_type0.materials[0] = LoadMaterialDefault();
        fmodels->rock_type0.materials[0].shader = gst->shaders[FOLIAGE_SHADER];
        fmodels->rock_type0.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture 
            = gst->textures[ROCK_TEXID];
 

        // Crystals
        fmodels->crystal = LoadModel("res/models/crystal.glb");
        fmodels->crystal.materials[0] = LoadMaterialDefault();
        fmodels->crystal.materials[0].shader = gst->shaders[CRYSTAL_FOLIAGE_SHADER];
 

    }


    _load_terrain_chunks(gst, terrain);


    // Figure out how many chunks may be rendered at once.
    // This is bad... but will do for now.

    terrain->num_max_visible_chunks = 8; // take in count error..
    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];

        float dst = Vector3Length(chunk->center_pos);

        if(dst <= RENDER_DISTANCE) {
            terrain->num_max_visible_chunks++;
        }
    }

    // Create plane for water
    const float waterplane_size = (CHUNK_SIZE * (1+terrain->num_max_visible_chunks/4)) * terrain->scaling;
    terrain->waterplane = LoadModelFromMesh(GenMeshPlane(waterplane_size, waterplane_size, 32, 32));
    terrain->waterplane.materials[0] = LoadMaterialDefault();
    terrain->waterplane.materials[0].shader = gst->shaders[WATER_SHADER];



    // Allocate memory for all foliage matrices.

    terrain->rfmatrices.tree_type0_size 
        = (terrain->num_max_visible_chunks * TREE_TYPE0_MAX_PERCHUNK);
    terrain->rfmatrices.tree_type0 
        = malloc(terrain->rfmatrices.tree_type0_size * sizeof(Matrix));

    terrain->rfmatrices.tree_type1_size 
        = (terrain->num_max_visible_chunks * TREE_TYPE1_MAX_PERCHUNK);
    terrain->rfmatrices.tree_type1 
        = malloc(terrain->rfmatrices.tree_type1_size * sizeof(Matrix));


    terrain->rfmatrices.rock_type0_size 
        = (terrain->num_max_visible_chunks * ROCK_TYPE0_MAX_PERCHUNK);
    terrain->rfmatrices.rock_type0 
        = malloc(terrain->rfmatrices.rock_type0_size * sizeof(Matrix));


    terrain->rfmatrices.crystals_size 
        = (terrain->num_max_visible_chunks * CRYSTALS_MAX_PERCHUNK);
    terrain->rfmatrices.crystals 
        = malloc(terrain->rfmatrices.crystals_size * sizeof(Matrix));


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

    if(terrain->rfmatrices.tree_type0) {
        free(terrain->rfmatrices.tree_type0);
        terrain->rfmatrices.tree_type0 = NULL;
    }
    if(terrain->rfmatrices.tree_type1) {
        free(terrain->rfmatrices.tree_type1);
        terrain->rfmatrices.tree_type1 = NULL;
    }
    if(terrain->rfmatrices.rock_type0) {
        free(terrain->rfmatrices.rock_type0);
        terrain->rfmatrices.rock_type0 = NULL;
    }
    if(terrain->rfmatrices.crystals) {
        free(terrain->rfmatrices.crystals);
        terrain->rfmatrices.crystals = NULL;
    }
    UnloadModel(terrain->waterplane);
    UnloadModel(terrain->foliage_models.tree_type0);
    UnloadModel(terrain->foliage_models.tree_type1);
    UnloadModel(terrain->foliage_models.rock_type0);
    UnloadModel(terrain->foliage_models.crystal);

    printf("\033[35m -> Deleted Terrain\033[0m\n");
}



static void copy_foliage_matrices_from_chunk(
        Matrix*   to_render_array,
        size_t    to_render_max_size,
        size_t*   to_render_array_elements,
        size_t*   to_render_array_index,
        Matrix*      chunk_fmatrices,     // fmatrices = 'foliage matrices'
        size_t       num_chunk_fmatrices
) {
    if(*to_render_array_index < to_render_max_size) {
        memmove(
                &to_render_array[*to_render_array_index],
                chunk_fmatrices,
                num_chunk_fmatrices * sizeof(Matrix)
                );
        *to_render_array_index += num_chunk_fmatrices;
        *to_render_array_elements += num_chunk_fmatrices;
    }
}

#include <rlgl.h>


static int chunk_in_player_view(struct player_t* player, struct terrain_t* terrain, struct chunk_t* chunk) {
    int res = 0;

    Vector3 P1 = (Vector3) {
        chunk->center_pos.x, 0, chunk->center_pos.z
    };

    Vector3 P2 = (Vector3) {
        player->position.x, 0, player->position.z
    };

    float test_dist = (terrain->chunk_size) * terrain->scaling;
    if(chunk->dst2player < test_dist) {
        res = 1;
        goto skip;
    }


    Vector3 up = (Vector3){ 0.0, 1.0, 0.0 };
    Vector3 right = GetCameraRight(&player->cam);
    Vector3 forward = Vector3CrossProduct(up, right);

    Vector3 dir = Vector3Normalize(Vector3Subtract(P1, P2));
    float dot = Vector3DotProduct(dir, forward);



    float f = map(dot, 1.0, -1.0, 0.0, 180.0);


    res = (f < 90.0);
skip:
    return res;
}



void render_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        int terrain_shader_index,
        int foliage_shader_index
){

    terrain->num_visible_chunks = 0;
    terrain->material.shader = gst->shaders[terrain_shader_index];

    // TODO: Clean this up.

    // - Clear foliage matrices.
    // - Copy matrices from chunk data to these arrays
    //   Then draw them all at once.

    memset(terrain->rfmatrices.tree_type0, 0, terrain->rfmatrices.tree_type0_size * sizeof(Matrix));
    terrain->rfmatrices.num_tree_type0 = 0;
    size_t rf_tree_type0_index = 0; // Where to copy from next chunk?

    memset(terrain->rfmatrices.tree_type1, 0, terrain->rfmatrices.tree_type1_size * sizeof(Matrix));
    terrain->rfmatrices.num_tree_type1 = 0;
    size_t rf_tree_type1_index = 0;


    memset(terrain->rfmatrices.rock_type0, 0, terrain->rfmatrices.rock_type0_size * sizeof(Matrix));
    terrain->rfmatrices.num_rock_type0 = 0;
    size_t rf_rock_type0_index = 0;


    memset(terrain->rfmatrices.crystals, 0, terrain->rfmatrices.crystals_size * sizeof(Matrix));
    terrain->rfmatrices.num_crystals = 0;
    size_t rf_crystal_index = 0;


    terrain->water_ylevel += sin(gst->time)*0.001585;

    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        chunk->dst2player = Vector3Distance(chunk->center_pos, gst->player.position);

        if(chunk->dst2player > RENDER_DISTANCE) {
            continue;
        }

        int inview = chunk_in_player_view(&gst->player, terrain, chunk);
        if(!inview) {
            continue;
        }

        terrain->num_visible_chunks++;

        copy_foliage_matrices_from_chunk(
                terrain->rfmatrices.tree_type0,
                terrain->rfmatrices.tree_type0_size,
                &terrain->rfmatrices.num_tree_type0,
                &rf_tree_type0_index,
                chunk->foliage_matrices.tree_type0,
                chunk->foliage_matrices.num_tree_type0
                );

        copy_foliage_matrices_from_chunk(
                terrain->rfmatrices.tree_type1,
                terrain->rfmatrices.tree_type1_size,
                &terrain->rfmatrices.num_tree_type1,
                &rf_tree_type1_index,
                chunk->foliage_matrices.tree_type1,
                chunk->foliage_matrices.num_tree_type1
                );

        copy_foliage_matrices_from_chunk(
                terrain->rfmatrices.rock_type0,
                terrain->rfmatrices.rock_type0_size,
                &terrain->rfmatrices.num_rock_type0,
                &rf_rock_type0_index,
                chunk->foliage_matrices.rock_type0,
                chunk->foliage_matrices.num_rock_type0
                );


        copy_foliage_matrices_from_chunk(
                terrain->rfmatrices.crystals,
                terrain->rfmatrices.crystals_size,
                &terrain->rfmatrices.num_crystals,
                &rf_crystal_index,
                chunk->foliage_matrices.crystals,
                chunk->foliage_matrices.num_crystals
                );


        Matrix translation = MatrixTranslate(chunk->position.x, 0, chunk->position.z);
        DrawMesh(terrain->chunks[i].mesh, terrain->material, translation);
    }

   
    terrain->foliage_models.tree_type0.materials[0].shader = gst->shaders[foliage_shader_index];
    terrain->foliage_models.tree_type0.materials[1].shader = gst->shaders[foliage_shader_index];
    terrain->foliage_models.tree_type1.materials[0].shader = gst->shaders[foliage_shader_index];
    terrain->foliage_models.tree_type1.materials[1].shader = gst->shaders[foliage_shader_index];
    terrain->foliage_models.rock_type0.materials[0].shader = gst->shaders[foliage_shader_index];
    
    if(foliage_shader_index == GBUFFER_INSTANCE_SHADER) {
        terrain->foliage_models.crystal.materials[0].shader = gst->shaders[foliage_shader_index];
    }
    else {
        terrain->foliage_models.crystal.materials[0].shader = gst->shaders[CRYSTAL_FOLIAGE_SHADER];
    }


    // Render all trees
    DrawMeshInstanced( // Tree bark
            terrain->foliage_models.tree_type0.meshes[0],
            terrain->foliage_models.tree_type0.materials[0],
            terrain->rfmatrices.tree_type0,
            terrain->rfmatrices.num_tree_type0
            );
    DrawMeshInstanced( // Tree leafs
            terrain->foliage_models.tree_type0.meshes[1],
            terrain->foliage_models.tree_type0.materials[1],
            terrain->rfmatrices.tree_type0,
            terrain->rfmatrices.num_tree_type0
            );

    DrawMeshInstanced( // Tree bark
            terrain->foliage_models.tree_type1.meshes[0],
            terrain->foliage_models.tree_type1.materials[0],
            terrain->rfmatrices.tree_type1,
            terrain->rfmatrices.num_tree_type1
            );
    DrawMeshInstanced( // Tree leafs
            terrain->foliage_models.tree_type1.meshes[1],
            terrain->foliage_models.tree_type1.materials[1],
            terrain->rfmatrices.tree_type1,
            terrain->rfmatrices.num_tree_type1
            );

    // Render all rocks
    DrawMeshInstanced(
            terrain->foliage_models.rock_type0.meshes[0],
            terrain->foliage_models.rock_type0.materials[0],
            terrain->rfmatrices.rock_type0,
            terrain->rfmatrices.num_rock_type0
            );


    // Render all crystals
    DrawMeshInstanced(
            terrain->foliage_models.crystal.meshes[0],
            terrain->foliage_models.crystal.materials[0],
            terrain->rfmatrices.crystals,
            terrain->rfmatrices.num_crystals
            );


    if(terrain_shader_index == DEFAULT_SHADER) {


        rlDisableBackfaceCulling();
        DrawModel(terrain->waterplane, 
                (Vector3){gst->player.position.x, terrain->water_ylevel, gst->player.position.z},
                1.0,
                (Color){ 255, 255, 255, 255 });
        
        rlEnableBackfaceCulling();
    }
 


}


