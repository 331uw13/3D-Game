#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "terrain.h"
#include "state.h"

#include "util.h"

#define TERRAIN_TRANSF_X -100
#define TERRAIN_TRANSF_Z -100


// ########  "Private" functions  ##############
// -----------------------------------------------------------
// NOTE: X and Z must be set accordingly with terrain X,Z positions and scaling.
// -----------------------------------------------------------
static size_t get_heightmap_index(struct terrain_t* terrain, float x, float z) {
    size_t index = 0;

    int r_x = round(x); 
    int r_z = round(z);

    long int i = (r_z * terrain->heightmap.size + r_x);

    i = (i < 0) ? 0 : (i > terrain->heightmap.total_size) 
        ? terrain->heightmap.total_size : i;
    index = (size_t)i;

    return index;
}


static float get_heightmap_value(struct terrain_t* terrain, float x, float z) {
    size_t index = get_heightmap_index(terrain, x, z);
    return terrain->heightmap.data[index];
}

// #############################################




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

Matrix get_rotation_to_surface(struct terrain_t* terrain, float x, float z, float* hit_y) {
    
    RayCollision t_hit = raycast_terrain(terrain, x, z);

    Vector3 up = (Vector3){ 0.0, 1.0, 0.0};
    Vector3 axis = Vector3CrossProduct(up, t_hit.normal);

    if(hit_y) {
        *hit_y = t_hit.point.y;
    }

    return MatrixRotateXYZ((Vector3){ axis.x, 0.0, axis.z });
}


static void _load_terrain_chunks(struct state_t* gst, struct terrain_t* terrain) {
    
    printf("--------- LOADING CHUNKS ---------\n\n");

    const int terrain_size = terrain->heightmap.size;

    terrain->chunk_size = 32;
    terrain->num_chunks 
        = (terrain_size / terrain->chunk_size)
        * (terrain_size / terrain->chunk_size);


    printf(" Terrain size: %i\n", terrain_size * terrain_size);
    printf(" Chunk size: %i\n", terrain->chunk_size * terrain->chunk_size);
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


    const int chunk_triangle_count = 2*((terrain->chunk_size-1) * (terrain->chunk_size-1));

    printf(" Chunk triangle count: %i\n", chunk_triangle_count);
    int chunk_x = 0;
    int chunk_z = 0;

    SetTraceLogLevel(LOG_WARNING);
    
    Vector3 scale = (Vector3) { terrain->scaling, 1.0, terrain->scaling };

    // Allocate memory for chunks and fill them with terrain vertices and normals.
    // Then they can be rendered individually.

    for(size_t i = 0; i < terrain->num_chunks; i++) {
        struct chunk_t* chunk = &terrain->chunks[i];
        *chunk = (struct chunk_t) { 0 };
        
        chunk->mesh.triangleCount = chunk_triangle_count;
        chunk->mesh.vertexCount = chunk->mesh.triangleCount * 3;

        chunk->mesh.vertices = malloc(chunk->mesh.vertexCount * 3 * sizeof(float));
        chunk->mesh.normals  = malloc(chunk->mesh.vertexCount * 3 * sizeof(float));
        /*TODO*/chunk->mesh.texcoords = malloc(chunk->mesh.vertexCount * 2 * sizeof(float));

        // Used for calculating normals.
        Vector3 vA = { 0 };
        Vector3 vB = { 0 };
        Vector3 vC = { 0 };
        Vector3 vD = { 0 };

        int v_counter = 0; // Count vertices.
        int n_counter = 0; // Count normals.

        for(int z = 0; z < terrain->chunk_size-1; z++) {
            for(int x = 0; x < terrain->chunk_size-1; x++) {
        
                // index relative to this chunk
                size_t chunk_index = (z * terrain->chunk_size + x);

                float X = (float)(x + chunk_x);
                float Z = (float)(z + chunk_z);

                // index in the whole terrain.
                //size_t terr_index =  (z+chunk_z) * terrain_size + (x+chunk_x);

                // Left up corner triangle

                chunk->mesh.vertices[v_counter]   = (float)X * scale.x;
                chunk->mesh.vertices[v_counter+1] = get_heightmap_value(terrain, X, Z);
                chunk->mesh.vertices[v_counter+2] = (float)Z * scale.z;
                
                chunk->mesh.vertices[v_counter+3] = (float)X * scale.x;
                chunk->mesh.vertices[v_counter+4] = get_heightmap_value(terrain, X, Z+1);
                chunk->mesh.vertices[v_counter+5] = (float)(Z+1) * scale.z;
                
                chunk->mesh.vertices[v_counter+6] = (float)(X+1) * scale.x;
                chunk->mesh.vertices[v_counter+7] = get_heightmap_value(terrain, X+1, Z);
                chunk->mesh.vertices[v_counter+8] = (float)Z * scale.z;


                // Right bottom corner triangle

                chunk->mesh.vertices[v_counter+9] = chunk->mesh.vertices[v_counter+6];
                chunk->mesh.vertices[v_counter+10] = chunk->mesh.vertices[v_counter+7];
                chunk->mesh.vertices[v_counter+11] = chunk->mesh.vertices[v_counter+8];
                
                chunk->mesh.vertices[v_counter+12] = chunk->mesh.vertices[v_counter+3];
                chunk->mesh.vertices[v_counter+13] = chunk->mesh.vertices[v_counter+4];
                chunk->mesh.vertices[v_counter+14] = chunk->mesh.vertices[v_counter+5];
                
                chunk->mesh.vertices[v_counter+15] = (float)(X+1) * scale.x;
                chunk->mesh.vertices[v_counter+16] = get_heightmap_value(terrain, X+1, Z+1);
                chunk->mesh.vertices[v_counter+17] = (float)(Z+1) * scale.z;

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

                n_counter += 18;
        /*


            for(int i = 0; i < 18; i+= 9) {
                vA.x = mesh->vertices[n_counter + i];
                vA.y = mesh->vertices[n_counter + i+1];
                vA.z = mesh->vertices[n_counter + i+2];
                
                vB.x = mesh->vertices[n_counter + i+3];
                vB.y = mesh->vertices[n_counter + i+4];
                vB.z = mesh->vertices[n_counter + i+5];
                
                vC.x = mesh->vertices[n_counter + i+6];
                vC.y = mesh->vertices[n_counter + i+7];
                vC.z = mesh->vertices[n_counter + i+8];

                vN = Vector3Normalize(
                        Vector3CrossProduct(
                            Vector3Subtract(vB, vA),
                            Vector3Subtract(vC, vA)
                            )
                        );

                mesh->normals[n_counter + i]   = vN.x;
                mesh->normals[n_counter + i+1] = vN.y;
                mesh->normals[n_counter + i+2] = vN.z;
                
                mesh->normals[n_counter + i+3] = vN.x;
                mesh->normals[n_counter + i+4] = vN.y;
                mesh->normals[n_counter + i+5] = vN.z;
                
                mesh->normals[n_counter + i+6] = vN.x;
                mesh->normals[n_counter + i+7] = vN.y;
                mesh->normals[n_counter + i+8] = vN.z;

            }

            */
            }
        }
        
        UploadMesh(&chunk->mesh, 0);

        chunk_x += terrain->chunk_size;
        if(chunk_x >= terrain_size) {
            chunk_x = 0;
            chunk_z += terrain->chunk_size;
        }

    }

    SetTraceLogLevel(LOG_ALL);

    printf("\n----------------\n");
}

static void _unload_chunks(struct terrain_t* terrain) {

    if(terrain->chunks) {

        for(size_t i = 0; i < terrain->num_chunks; i++) {
            UnloadMesh(terrain->chunks[i].mesh);
        }

        free(terrain->chunks);
        terrain->chunks = NULL;
    
        printf("----- UNLOADED CHUNKS -----\n");
    }
}


void generate_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        u32    terrain_size,
        float  terrain_scaling,
        float  amplitude,
        float  frequency,
        int    octaves
) {

    // start by generating a height map for the terrain.
    
    terrain->heightmap.total_size = terrain_size * terrain_size;
    terrain->heightmap.data = malloc(terrain->heightmap.total_size * sizeof *terrain->heightmap.data);
    terrain->heightmap.size = terrain_size;
    terrain->chunks = NULL;

    terrain->highest_point = 0.0;

    // First pass.
    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = round(z) * terrain->heightmap.size + round(x);

            float p_nx = ((float)x / (float)terrain->heightmap.size) * frequency;
            float p_nz = ((float)z / (float)terrain->heightmap.size) * frequency;

            float value = fbm_2D(p_nx, p_nz, octaves) * amplitude;
            terrain->heightmap.data[index] = value;
        }
    }

    // Second pass.
    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = round(z) * terrain->heightmap.size + round(x);

            float p_nx = ((float)x / (float)terrain->heightmap.size) * (frequency/3);
            float p_nz = ((float)z / (float)terrain->heightmap.size) * (frequency/3);

            float value = fbm_2D(p_nx, p_nz, octaves) * (amplitude * 3);
            terrain->heightmap.data[index] += value;
        }
    }


    // Get highest point.
    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = round(z) * terrain->heightmap.size + round(x);

            float v = terrain->heightmap.data[index];
            if(v > terrain->highest_point) {
                terrain->highest_point = v;
            }
        }
    }
    

    // then generate triangles for the mesh.


    Mesh* mesh = &terrain->mesh;


    mesh->triangleCount = (terrain->heightmap.size-1) * (terrain->heightmap.size-1) * 2;
    mesh->vertexCount = mesh->triangleCount * 3;

    mesh->vertices = malloc(mesh->vertexCount * 3 * sizeof(float));
    mesh->normals  = malloc(mesh->vertexCount * 3 * sizeof(float));
    mesh->texcoords = malloc(mesh->vertexCount * 2 * sizeof(float));

    terrain->triangle_lookup = malloc(mesh->vertexCount * sizeof *terrain->triangle_lookup);
    

    int v_counter = 0;  // count mesh vertices.
    int n_counter = 0;  // count mesh normals.
    int tc_counter = 0; // count mesh texture coordinates.

    // TODO:
    Vector3 scale = (Vector3) { terrain_scaling, 1.0, terrain_scaling };

    // used for calculating normals
    Vector3 vA = { 0 };
    Vector3 vC = { 0 };
    Vector3 vB = { 0 };
    Vector3 vN = { 0 };

    for(u32 z = 0; z < terrain->heightmap.size-1; z++) {
        for(u32 x = 0; x < terrain->heightmap.size-1; x++) {


            // one triangle
            mesh->vertices[v_counter]   = (float)x * scale.x;
            mesh->vertices[v_counter+1] = get_heightmap_value(terrain, x, z);
            mesh->vertices[v_counter+2] = (float)z * scale.z;

            mesh->vertices[v_counter+3] = (float)x * scale.x;
            mesh->vertices[v_counter+4] = get_heightmap_value(terrain, x, z+1);
            mesh->vertices[v_counter+5] = (float)(z+1) * scale.z;

            mesh->vertices[v_counter+6] = (float)(x+1) * scale.x;
            mesh->vertices[v_counter+7] = get_heightmap_value(terrain, x+1, z);
            mesh->vertices[v_counter+8] = (float)z * scale.z;


            // another triangle
            mesh->vertices[v_counter+9]  = mesh->vertices[v_counter+6];
            mesh->vertices[v_counter+10] = mesh->vertices[v_counter+7];
            mesh->vertices[v_counter+11] = mesh->vertices[v_counter+8];
            
            mesh->vertices[v_counter+12] = mesh->vertices[v_counter+3];
            mesh->vertices[v_counter+13] = mesh->vertices[v_counter+4];
            mesh->vertices[v_counter+14] = mesh->vertices[v_counter+5];

            mesh->vertices[v_counter+15] = (float)(x+1) * scale.x;
            mesh->vertices[v_counter+16] = get_heightmap_value(terrain, x+1, z+1);
            mesh->vertices[v_counter+17] = (float)(z+1) * scale.z;


            // save the triangle into triangle_lookup array.
            //
            size_t tr_index = (z * terrain->heightmap.size) + x;

            terrain->triangle_lookup[tr_index] = (struct triangle2x_t) {
            
                .a0 = (Vector3) {
                    mesh->vertices[v_counter],
                    mesh->vertices[v_counter+1],
                    mesh->vertices[v_counter+2]
                },
                .a1 = (Vector3) {
                    mesh->vertices[v_counter+3],
                    mesh->vertices[v_counter+4],
                    mesh->vertices[v_counter+5]
                },
                .a2 = (Vector3) {
                    mesh->vertices[v_counter+6],
                    mesh->vertices[v_counter+7],
                    mesh->vertices[v_counter+8]
                },


                .b0 = (Vector3) {
                    mesh->vertices[v_counter+9],
                    mesh->vertices[v_counter+10],
                    mesh->vertices[v_counter+11]
                },
                .b1 = (Vector3) {
                    mesh->vertices[v_counter+12],
                    mesh->vertices[v_counter+13],
                    mesh->vertices[v_counter+14]
                },
                .b2 = (Vector3) {
                    mesh->vertices[v_counter+15],
                    mesh->vertices[v_counter+16],
                    mesh->vertices[v_counter+17]
                }
            };


            v_counter += 18;



            for(int i = 0; i < 18; i+= 9) {
                vA.x = mesh->vertices[n_counter + i];
                vA.y = mesh->vertices[n_counter + i+1];
                vA.z = mesh->vertices[n_counter + i+2];
                
                vB.x = mesh->vertices[n_counter + i+3];
                vB.y = mesh->vertices[n_counter + i+4];
                vB.z = mesh->vertices[n_counter + i+5];
                
                vC.x = mesh->vertices[n_counter + i+6];
                vC.y = mesh->vertices[n_counter + i+7];
                vC.z = mesh->vertices[n_counter + i+8];

                vN = Vector3Normalize(
                        Vector3CrossProduct(
                            Vector3Subtract(vB, vA),
                            Vector3Subtract(vC, vA)
                            )
                        );

                mesh->normals[n_counter + i]   = vN.x;
                mesh->normals[n_counter + i+1] = vN.y;
                mesh->normals[n_counter + i+2] = vN.z;
                
                mesh->normals[n_counter + i+3] = vN.x;
                mesh->normals[n_counter + i+4] = vN.y;
                mesh->normals[n_counter + i+5] = vN.z;
                
                mesh->normals[n_counter + i+6] = vN.x;
                mesh->normals[n_counter + i+7] = vN.y;
                mesh->normals[n_counter + i+8] = vN.z;

            }

            n_counter += 18;
        }
    }

    terrain->scaling = terrain_scaling;
    UploadMesh(mesh, 0);

    float terrain_pos = -(terrain_size * terrain_scaling) / 2;
    terrain->transform = MatrixTranslate(terrain_pos, 0, terrain_pos);
    terrain->material = LoadMaterialDefault();
    terrain->material.shader = gst->shaders[DEFAULT_SHADER];


    _load_terrain_chunks(gst, terrain);


    printf("\033[32m >> Generated terrain succesfully.\033[0m\n");

    terrain->mesh_generated = 1;
}

void generate_terrain_foliage(struct state_t* gst, struct terrain_t* terrain) {
    if(!terrain->mesh_generated) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Terrain must be generated first.\033[0m\n",
                __func__);
        return;
    }

    // Create tree type0

    terrain->foliage.tree0_model = LoadModel("res/models/tree.glb");
    terrain->foliage.tree0_material = LoadMaterialDefault();
    terrain->foliage.tree0_material.shader = gst->shaders[FOLIAGE_SHADER];
    terrain->foliage.tree0_material.maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[TREEBARK_TEXID];

    const float tscale = terrain->heightmap.size * terrain->scaling;

    for(int i = 0; i < NUM_TREE_TYPE0; i++) {
    
        float x = RSEEDRANDOMF(-tscale, tscale);
        float z = RSEEDRANDOMF(-tscale, tscale);

        RayCollision ray = raycast_terrain(terrain, x, z);
        Vector3 pos = (Vector3) {
            x, ray.point.y, z
        };

        Matrix transform = MatrixTranslate(pos.x, pos.y, pos.z);
        Matrix rotation = MatrixRotateY(RSEEDRANDOMF(0, 360) * DEG2RAD);

        transform = MatrixMultiply(rotation, transform);

        terrain->foliage.tree0_transforms[i] = transform;
    }
}


void delete_terrain_foliage(struct terrain_t* terrain) {
    UnloadModel(terrain->foliage.tree0_model);

}

void delete_terrain(struct terrain_t* terrain) {

    /*
    if(terrain->mesh.vertices) {
        free(terrain->mesh.vertices);
        terrain->mesh.vertices = NULL;
    }
    if(terrain->mesh.normals) {
        free(terrain->mesh.normals);
        terrain->mesh.normals = NULL;
    }
    if(terrain->mesh.texcoords) {
        free(terrain->mesh.texcoords);
        terrain->mesh.texcoords = NULL;
    }
    */
    if(terrain->triangle_lookup) {
        free(terrain->triangle_lookup);
        terrain->triangle_lookup = NULL;
    }
    if(terrain->heightmap.data) {
        free(terrain->heightmap.data);
        terrain->heightmap.data = NULL;
    }

    _unload_chunks(terrain);
   
    UnloadMesh(terrain->mesh);
    
    terrain->mesh_generated = 0;
    printf("\033[32m >> Deleted terrain.\033[0m\n");
}


void render_terrain(struct state_t* gst, struct terrain_t* terrain) {
 
    /*
    if(terrain->mesh_generated) {
        DrawMesh(terrain->mesh, terrain->material, terrain->transform);
    }
    */

    Matrix translation = MatrixTranslate(0, 3, 0);
    DrawMesh(terrain->chunks[0].mesh, terrain->material, translation);

}


