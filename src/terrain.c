#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "terrain.h"
#include "state.h"
#include <raymath.h>

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

    //terrain->xz_scale = 1.0;


    terrain->highest_point = 0.0;

    for(u32 z = 0; z < terrain->heightmap.size; z++) {
        for(u32 x = 0; x < terrain->heightmap.size; x++) {
            size_t index = round(z) * terrain->heightmap.size + round(x);

            float p_nx = ((float)x / (float)terrain->heightmap.size) * frequency;
            float p_nz = ((float)z / (float)terrain->heightmap.size) * frequency;

            float value = fbm_2D(p_nx, p_nz, octaves) * amplitude;

            if(value > terrain->highest_point) {
                terrain->highest_point = value;
            }

            terrain->heightmap.data[index] = value;
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

    printf("\033[32m >> Generated terrain succesfully.\033[0m\n");

    terrain->mesh_generated = 1;
}

void delete_terrain(struct terrain_t* terrain) {

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
    if(terrain->triangle_lookup) {
        free(terrain->triangle_lookup);
        terrain->triangle_lookup = NULL;
    }
    if(terrain->heightmap.data) {
        free(terrain->heightmap.data);
        terrain->heightmap.data = NULL;
    }

   
    UnloadMesh(terrain->mesh);
    
    terrain->mesh_generated = 0;
    printf("\033[32m >> Deleted terrain.\033[0m\n");
}


void render_terrain(struct state_t* gst, struct terrain_t* terrain) {
  
    if(terrain->mesh_generated) {
        DrawMesh(terrain->mesh, terrain->material, terrain->transform);
    }

}


