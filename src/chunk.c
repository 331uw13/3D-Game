#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "state/state.h"
#include "state/state_render.h"

#include <rlgl.h>


static void load_foliage_model(
        struct state_t* gst,
        Model* modelptr,
        const char* model_filepath
){
    if(!FileExists(model_filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': \"%s\" Not found.\033[0m\n",
                __func__, model_filepath);
        return;
    }

    *modelptr = LoadModel(model_filepath);
    
    for(int i = 0; i < modelptr->materialCount; i++) {
        modelptr->materials[i] = LoadMaterialDefault();
        modelptr->materials[i].shader = gst->shaders[FOLIAGE_SHADER];
    }
}

static void set_foliage_texture(
        struct state_t* gst,
        int foliage_index,
        int material_index,
        int texture_index
){
    gst->terrain.foliage_models[foliage_index]
        .materials[material_index].maps[MATERIAL_MAP_DIFFUSE]
        .texture = gst->textures[texture_index];
}

static void finish_chunk_setup(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

    chunk->area = (struct chunk_area_t) {
        .x_min = chunk->position.x,
        .x_max = chunk->position.x + (terrain->chunk_size * terrain->scaling),

        .z_min = chunk->position.z,
        .z_max = chunk->position.z + (terrain->chunk_size * terrain->scaling)
    };
    
    decide_chunk_biome(gst, terrain, chunk);
    load_chunk_foliage(gst, terrain, chunk);
   
}


void load_foliage_models(struct state_t* gst, struct terrain_t* terrain) {
    SetTraceLogLevel(LOG_ALL);

    // BIOMEID_COMFY

    terrain->foliage_max_perchunk[TF_COMFY_TREE_0] = 64;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_TREE_0], "res/models/biomes/comfy/tree_type0.glb");
    set_foliage_texture(gst, TF_COMFY_TREE_0, 0, TREEBARK_TEXID);
    set_foliage_texture(gst, TF_COMFY_TREE_0, 1, LEAF_TEXID);

    terrain->foliage_max_perchunk[TF_COMFY_TREE_1] = 32;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_TREE_1], "res/models/biomes/comfy/tree_type1.glb");
    set_foliage_texture(gst, TF_COMFY_TREE_1, 0, TREEBARK_TEXID);
    set_foliage_texture(gst, TF_COMFY_TREE_1, 1, LEAF_TEXID);


    terrain->foliage_max_perchunk[TF_COMFY_ROCK_0] = 16;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_ROCK_0], "res/models/biomes/comfy/rock_type0.glb");
    set_foliage_texture(gst, TF_COMFY_ROCK_0, 0, ROCK_TEXID);


    terrain->foliage_max_perchunk[TF_COMFY_MUSHROOM_0] = 500;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_MUSHROOM_0], "res/models/biomes/comfy/mushroom.glb");
    set_foliage_texture(gst, TF_COMFY_MUSHROOM_0, 0, TERRAIN_MUSHROOM_TEXID);



    // BIOMEID_HAZY

    terrain->foliage_max_perchunk[TF_HAZY_TREE_0] = 45;
    load_foliage_model(gst, &terrain->foliage_models[TF_HAZY_TREE_0], "res/models/biomes/hazy/tree_type0.glb");
    set_foliage_texture(gst, TF_HAZY_TREE_0, 0, TREEBARK_TEXID);

    terrain->foliage_max_perchunk[TF_HAZY_ROCK_0] = 45;
    load_foliage_model(gst, &terrain->foliage_models[TF_HAZY_ROCK_0], "res/models/biomes/hazy/rock_type0.glb");
    set_foliage_texture(gst, TF_HAZY_ROCK_0, 0, ROCK_TEXID);



    SetTraceLogLevel(LOG_NONE);
}


void delete_chunk(struct chunk_t* chunk) {
    for(int j = 0; j < MAX_FOLIAGE_TYPES; j++) {
        free(chunk->foliage_data[j].matrices);
    }
    UnloadMesh(chunk->mesh);
}



void load_chunk(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        int chunk_x,
        int chunk_z,
        int chunk_triangle_count
){
    chunk->mesh.triangleCount = chunk_triangle_count;
    chunk->mesh.vertexCount = chunk->mesh.triangleCount * 3;

    chunk->mesh.vertices = malloc(chunk->mesh.vertexCount * 3 * sizeof(float));
    chunk->mesh.normals  = malloc(chunk->mesh.vertexCount * 3 * sizeof(float));
    chunk->mesh.texcoords = malloc(chunk->mesh.vertexCount * 2 * sizeof(float));

    int terrain_size = terrain->heightmap.size;

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

    Vector3 scale = (Vector3){ terrain->scaling, 0.0, terrain->scaling };

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

  
    UploadMesh(&chunk->mesh, 0);

    finish_chunk_setup(gst, terrain, chunk);
}




#define NO_PERLIN_NOISE 0
#define USE_PERLIN_NOISE 1


static void fill_chunk_fdata_matrices(
        struct state_t* gst,
        int  foliage_index,
        struct chunk_t*      chunk,

        Vector3 position_offset,
        Vector3 perinstance_rotation,
        Vector3 rotation, // Rotation for all instances.

        int     use_perlin_noise,
        Vector2 perlin_noise_scale,
        float   perlin_noise_tresh
){

    struct chunk_foliage_data_t* chunk_fdata = &chunk->foliage_data[foliage_index];
    
    for(size_t i = 0; i < chunk_fdata->matrices_size; i++) {
        float x = RSEEDRANDOMF(chunk->area.x_min, chunk->area.x_max);
        float z = RSEEDRANDOMF(chunk->area.z_min, chunk->area.z_max);

        if(use_perlin_noise) {
            float pn = perlin_noise_2D(x*perlin_noise_scale.x, z*perlin_noise_scale.y);
            pn *= 2.0;

            //printf("%f\n", pn);
            if(pn < perlin_noise_tresh) {
                continue;
            }
        }

        RayCollision ray = raycast_terrain(&gst->terrain, x, z);
        
        Matrix translation = MatrixTranslate(
                position_offset.x + x,
                position_offset.y + ray.point.y,
                position_offset.z + z
        );
        Matrix rotation_matrix = MatrixRotateXYZ((Vector3){ 
            RSEEDRANDOMF(-M_PI, M_PI) * perinstance_rotation.x,
            RSEEDRANDOMF(-M_PI, M_PI) * perinstance_rotation.y,
            RSEEDRANDOMF(-M_PI, M_PI) * perinstance_rotation.z
        });

        rotation_matrix = MatrixMultiply(rotation_matrix, MatrixRotateXYZ(rotation));

        chunk_fdata->matrices[chunk_fdata->num_foliage] = MatrixMultiply(rotation_matrix, translation);
        chunk_fdata->num_foliage++;
    }
}


void load_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

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
    
        terrain->foliage_rdata[i].render_backface = 0;
    }


    switch(chunk->biome.id) {
    
        case BIOMEID_COMFY:
            {
                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_TREE_0,
                        chunk,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 0, 1, 0 }, // Random rotation per instance
                        (Vector3){ 0, 0, 0 }, // Random rotation for all instances.
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );


                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_TREE_1,
                        chunk,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 0, 1, 0 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );

                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_ROCK_0,
                        chunk,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 1, 1, 1 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );

                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_MUSHROOM_0,
                        chunk,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 0, 1, 0 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        USE_PERLIN_NOISE, (Vector2){ 0.00025, 0.00025 }, 0.0
                        );

            }
            break;

        case BIOMEID_HAZY:
            {
                fill_chunk_fdata_matrices(gst, 
                        TF_HAZY_TREE_0,
                        chunk,  
                        (Vector3){ 0, 0, 0 },   // Position offset
                        (Vector3){ 0.075, 1, 0.075 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );

                fill_chunk_fdata_matrices(gst, 
                        TF_HAZY_ROCK_0,
                        chunk,
                        (Vector3){ 0, 0, 0 },   // Position offset
                        (Vector3){ 0.075, 1, 0.075 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        USE_PERLIN_NOISE, (Vector2){ 0.005, 0.005 }, 0.0
                        );
            }
            break;

        case BIOMEID_EVIL:
            // No models have been made yet.
            break;
    }
}

void decide_chunk_biome(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

    //RayCollision ray = raycast_terrain(terrain, chunk->center_pos.x, chunk->center_pos.z);
    int biomeid = get_biomeid_by_ylevel(gst, chunk->center_pos.y);
    chunk->biome = terrain->biomedata[biomeid];
}

void render_chunk_borders(struct state_t* gst, struct chunk_t* chunk, Color color) {
    float scale = gst->terrain.chunk_size * gst->terrain.scaling;
    Vector3 size = (Vector3){ scale, scale, scale };
    Vector3 box = (Vector3){ chunk->center_pos.x, chunk->position.y, chunk->center_pos.z };
    DrawCubeWiresV(box, size, color);
}

void render_chunk_borders2x(struct state_t* gst, struct chunk_t* chunk, Color color) {
    float scale = gst->terrain.chunk_size * gst->terrain.scaling;
    Vector3 size = (Vector3){ scale*0.5, scale*0.5, scale*0.5 };
    Vector3 box = (Vector3){ chunk->center_pos.x, chunk->position.y, chunk->center_pos.z };
    DrawCubeWiresV(box, size, color);
}

struct chunk_t* find_chunk(struct state_t* gst, Vector3 position) {
    struct chunk_t* chunk = NULL;
    for(size_t i = 0; i < gst->terrain.num_chunks; i++) {
        chunk = &gst->terrain.chunks[i];
        if((position.x > chunk->area.x_min) && (position.x < chunk->area.x_max)
        && (position.z > chunk->area.z_min) && (position.z < chunk->area.z_max)) {
            break;
        }
    }

    return chunk;
}


void render_chunk_grass(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        Matrix* mvp,
        int render_pass
){


    int mesh_triangle_count = terrain->grass_model.meshes[0].triangleCount;
    int vao_id = terrain->grass_model.meshes[0].vaoId;
    
    int baseindex = (int)chunk->grass_baseindex;
    int instances = terrain->grass_instances_perchunk;

    if(chunk->dst2player > 7500) {
        return;
    }

    if(chunk->dst2player > 1.15*(terrain->chunk_size * terrain->scaling)) {
        // Lower resolution is chosen for far away chunks.

        instances = terrain->grass_instances_perchunk / 3.5;
        mesh_triangle_count = terrain->grass_model_lowres.meshes[0].triangleCount;
        vao_id = terrain->grass_model_lowres.meshes[0].vaoId;
    }
    

    // Input for compute shader
    {
        // Base index.
        shader_setu_int(gst, 
                GRASSDATA_COMPUTE_SHADER,
                U_CHUNK_GRASS_BASEINDEX,
                &baseindex
                );

        // Chunk coords.
        Vector2 chunk_coords = (Vector2){ chunk->position.x, chunk->position.z };
        shader_setu_vec2(gst,
                GRASSDATA_COMPUTE_SHADER,
                U_CHUNK_COORDS,
                &chunk_coords);

       
        shader_setu_int(gst,
                GRASSDATA_COMPUTE_SHADER,
                U_NUM_GRASS_PERCHUNK,
                ((int*)&terrain->grass_instances_perchunk));

        // Chunk size.
        shader_setu_int(gst,
                GRASSDATA_COMPUTE_SHADER,
                U_CHUNK_SIZE,
                &terrain->chunk_size);
    
    }

    // Dispatch grassdata compute shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[GRASSDATA_SSBO]);
    dispatch_compute(gst,
            GRASSDATA_COMPUTE_SHADER,
            instances,  1, 1,
            GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
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


