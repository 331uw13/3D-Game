#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "terrain.h"
#include "state.h"
#include <raymath.h>



size_t get_heightmap_index(struct terrain_t* terrain, float x, float z) {
    size_t index = 0;

    int r_x = round(x / terrain->xz_scale);
    int r_z = round(z / terrain->xz_scale);

    long int i = (r_z * terrain->heightmap.size_x + r_x);

    i = (i < 0) ? 0 : (i > HEIGHTMAP_SIZE_XZ) ? HEIGHTMAP_SIZE_XZ : i;
    index = (size_t)i;


    return index;
}


float get_heightmap_value(struct terrain_t* terrain, float x, float z) {
    size_t index = get_heightmap_index(terrain, x, z);
    return terrain->heightmap.data[index];
}



void generate_heightmap(struct terrain_t* terrain) {

    memset(terrain->heightmap.data, 0, sizeof(float) * HEIGHTMAP_SIZE_XZ);
    terrain->heightmap.ready = 0;

    terrain->heightmap.size_x = HEIGHTMAP_SIZE_X;
    terrain->heightmap.size_z = HEIGHTMAP_SIZE_Z;

    terrain->xz_scale = 1.0;

    float max_x = (float)terrain->heightmap.size_x;
    float max_z = (float)terrain->heightmap.size_z;


    float freq = 8.0;
    float amp = 8.0;

    terrain->highest_point = 0.0;

    for(int z = 0; z < terrain->heightmap.size_z; z++) {
        for(int x = 0; x < terrain->heightmap.size_x; x++) {
            size_t index = get_heightmap_index(terrain, (float)x, (float)z);

            float p_nx = ((float)x / max_x) * freq;
            float p_nz = ((float)z / max_z) * freq;

            float value = fbm_2D(p_nx, p_nz, 3) * amp;

            if(value > terrain->highest_point) {
                terrain->highest_point = value;
            }

            terrain->heightmap.data[index] = value;
        }
    }

    /*
    // FOR TESTING.

    
    const Vector2 heightmap_center = (Vector2) { 
        terrain->heightmap.size_x / 2,
        terrain->heightmap.size_z / 2
    };

    float test = 0.0;

    for(int z = 0; z < terrain->heightmap.size_z; z++) {
        for(int x = 0; x < terrain->heightmap.size_x; x++) {
            size_t index = get_heightmap_index(terrain, (float)x, (float)z);

            Vector2 current = (Vector2) { (float)x, (float)z };

            float t = cos(x)*0.5 + sin(z)*0.5;
            terrain->heightmap.data[index] 
                = t + 6.0 * Vector2Distance(current, heightmap_center)
                    / (float)terrain->heightmap.size_x;
        

        } 
    }

    // ----------
*/

    terrain->heightmap.ready = 1;
}


void generate_terrain_mesh(struct state_t* gst, struct terrain_t* terrain) {

    if(!terrain->heightmap.ready) {
        fprintf(stderr, " >> (ERROR) '%s': heightmap for terrain is not generated?\n",
                __func__);
        return;
    }

    Mesh* mesh = &terrain->mesh;

    const float sxz = 4.0;

    mesh->triangleCount = (terrain->heightmap.size_x-1) * (terrain->heightmap.size_z-1) * 2;
    mesh->vertexCount = mesh->triangleCount * 3;

    mesh->vertices = malloc(mesh->vertexCount * 3 * sizeof(float));
    mesh->normals  = malloc(mesh->vertexCount * 3 * sizeof(float));
    mesh->texcoords = malloc(mesh->vertexCount * 2 * sizeof(float));

    int v_counter = 0;  // count mesh vertices.
    int n_counter = 0;  // count mesh normals.
    int tc_counter = 0; // count mesh texture coordinates.

    // TODO:
    Vector3 scale = (Vector3) { sxz, 1.0, sxz };

    // used for calculating normals
    Vector3 vA = { 0 };
    Vector3 vC = { 0 };
    Vector3 vB = { 0 };
    Vector3 vN = { 0 };

    for(int z = 0; z < terrain->heightmap.size_z-1; z++) {
        for(int x = 0; x < terrain->heightmap.size_x-1; x++) {


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

    terrain->xz_scale = sxz;
    UploadMesh(mesh, 0);

    terrain->transform = MatrixTranslate(0, 0, 0);
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

    terrain->mesh_generated = 0;
   

    printf("\033[32m >> Deleted terrain.\033[0m\n");
}


void render_terrain(struct state_t* gst, struct terrain_t* terrain) {
  
    if(terrain->mesh_generated) {
        DrawMesh(terrain->mesh, terrain->material, terrain->transform);
    }

}


