#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "state/state.h"

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

struct chunk_area_t {
    float x_min;
    float x_max;
    float z_min;
    float z_max;
};

#define NO_PERLIN_NOISE 0
#define USE_PERLIN_NOISE 1


static void fill_chunk_fdata_matrices(
        struct state_t* gst,
        int  foliage_index,
        struct chunk_t*      chunk,
        struct chunk_area_t* chunk_area,

        Vector3 position_offset,
        Vector3 perinstance_rotation,
        Vector3 rotation, // Rotation for all instances.

        int     use_perlin_noise,
        Vector2 perlin_noise_scale,
        float   perlin_noise_tresh
){

    struct chunk_foliage_data_t* chunk_fdata = &chunk->foliage_data[foliage_index];
    
    for(size_t i = 0; i < chunk_fdata->matrices_size; i++) {
        float x = RSEEDRANDOMF(chunk_area->x_min, chunk_area->x_max);
        float z = RSEEDRANDOMF(chunk_area->z_min, chunk_area->z_max);

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


static void load_chunk_grassdata(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        struct chunk_area_t* chunk_area
){
    

    chunk->grassdata.vbo = 0;
    chunk->grassdata.vao = 0;
    chunk->grassdata.vertices = NULL;

    const size_t num_blades = 100000;
    const size_t vertices_size = (num_blades * 3) * sizeof(float);
    chunk->grassdata.vertices = malloc(vertices_size);
    chunk->grassdata.num_vertices = num_blades;

    for(size_t i = 0; i < num_blades; i += 3) {

        float* x = &chunk->grassdata.vertices[i+0];
        float* y = &chunk->grassdata.vertices[i+1];
        float* z = &chunk->grassdata.vertices[i+2];

        // TODO: To get better results, create grid for grass blade positions
        //       then add small offset to the blades. But this will do for testing.
        float rnd_x = RSEEDRANDOMF(chunk_area->x_min, chunk_area->x_max);
        float rnd_z = RSEEDRANDOMF(chunk_area->z_min, chunk_area->z_max);


        *y = raycast_terrain(terrain, rnd_x, rnd_z).point.y + 5.0;
        *x = rnd_x;
        *z = rnd_z;
    }

    chunk->grassdata.vao = rlLoadVertexArray();
    rlEnableVertexArray(chunk->grassdata.vao);

    chunk->grassdata.vbo = rlLoadVertexBuffer(
                chunk->grassdata.vertices,
                vertices_size,
                0 // No dynamic vbo.
            );

    rlEnableVertexBuffer(chunk->grassdata.vbo);

    // Grass blades only need position. all other data is created with shaders.
    int attr_index = 0;
    rlEnableVertexAttribute(attr_index);
    rlSetVertexAttribute(attr_index, 3, RL_FLOAT, 0, sizeof(float)*3, 0);


    rlDisableVertexBuffer();
    rlDisableVertexArray();
}

void delete_chunk(struct chunk_t* chunk) {
   
    if(chunk->grassdata.vertices) {
        free(chunk->grassdata.vertices);
    }

    glDeleteBuffers(1, &chunk->grassdata.vbo);
    glDeleteVertexArrays(1, &chunk->grassdata.vao);

    // TODO: Delete grass data VAO and VBO.

    UnloadMesh(chunk->mesh);
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

    const float x_min = chunk->position.x;
    const float x_max = chunk->position.x + (terrain->chunk_size * terrain->scaling);
    const float z_min = chunk->position.z;
    const float z_max = chunk->position.z + (terrain->chunk_size * terrain->scaling);

    struct chunk_area_t chunk_area = (struct chunk_area_t) {
        x_min, x_max,
        z_min, z_max
    };


    struct chunk_foliage_data_t* chunk_fdata = NULL;
    load_chunk_grassdata(gst, terrain, chunk, &chunk_area);
    
    switch(chunk->biome.id) {
    
        case BIOMEID_COMFY:
            {
                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_TREE_0,
                        chunk, &chunk_area,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 0, 1, 0 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );


                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_TREE_1,
                        chunk, &chunk_area,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 0, 1, 0 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );

                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_ROCK_0,
                        chunk, &chunk_area,
                        (Vector3){ 0, 0, 0 }, // Position offset
                        (Vector3){ 1, 1, 1 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );

                fill_chunk_fdata_matrices(gst, 
                        TF_COMFY_MUSHROOM_0,
                        chunk, &chunk_area,
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
                        chunk, &chunk_area,  
                        (Vector3){ 0, 0, 0 },   // Position offset
                        (Vector3){ 0.075, 1, 0.075 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        NO_PERLIN_NOISE, (Vector2){ 0, 0 }, 0
                        );

                fill_chunk_fdata_matrices(gst, 
                        TF_HAZY_ROCK_0,
                        chunk, &chunk_area,
                        (Vector3){ 0, 0, 0 },   // Position offset
                        (Vector3){ 0.075, 1, 0.075 }, // Rotation
                        (Vector3){ 0, 0, 0 },
                        USE_PERLIN_NOISE, (Vector2){ 0.005, 0.005 }, 0.0
                        );
            }
            break;
    }
}

void decide_chunk_biome(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

    //RayCollision ray = raycast_terrain(terrain, chunk->center_pos.x, chunk->center_pos.z);
    int biomeid = get_biomeid_by_ylevel(gst, chunk->center_pos.y);
    chunk->biome = terrain->biomedata[biomeid];
}


