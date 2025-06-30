#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    setup_chunk_foliage(gst, terrain, chunk);

    chunk->num_enemies = 0;
    for(int i = 0; i < MAX_ENEMIES_PERCHUNK; i++) {
        chunk->enemies[i] = (struct enemy_t){ 0 };
    }
}

void load_chunk_foliage_models(struct state_t* gst, struct terrain_t* terrain) {
    SetTraceLogLevel(LOG_ALL);

    // BIOMEID_COMFY

    terrain->foliage_max_perchunk[TF_COMFY_TREE_0] = 16;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_TREE_0], "res/models/biomes/comfy/tree_type0.glb");
    set_foliage_texture(gst, TF_COMFY_TREE_0, 0, TREEBARK_TEXID);
    set_foliage_texture(gst, TF_COMFY_TREE_0, 1, LEAF_TEXID);

    terrain->foliage_max_perchunk[TF_COMFY_TREE_1] = 14;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_TREE_1], "res/models/biomes/comfy/tree_type1.glb");
    set_foliage_texture(gst, TF_COMFY_TREE_1, 0, TREEBARK_TEXID);
    set_foliage_texture(gst, TF_COMFY_TREE_1, 1, LEAF_TEXID);


    terrain->foliage_max_perchunk[TF_COMFY_ROCK_0] = 8;
    load_foliage_model(gst, &terrain->foliage_models[TF_COMFY_ROCK_0], "res/models/biomes/comfy/rock_type0.glb");
    set_foliage_texture(gst, TF_COMFY_ROCK_0, 0, ROCK_TEXID);


    terrain->foliage_max_perchunk[TF_COMFY_MUSHROOM_0] = 400;
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
    for(int i = 0; i < chunk->num_fractals; i++) {
        delete_fractal_model(&chunk->fractals[i]);
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

    chunk->num_items = 0;
   

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


void setup_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk) {

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
   

    // Generate Fractals.

    chunk->num_fractals = 0;
    if(RSEEDRANDOMF(0, 100) < FRACTAL_SPAWN_CHANCE) {
        
        chunk->num_fractals = GetRandomValue(1, MAX_FRACTALS_PERCHUNK-1);
        
        for(int i = 0; i < chunk->num_fractals; i++) {
            struct fractal_t* fractal = &chunk->fractals[i];
            
            Vector3 pos = (Vector3) {
                RSEEDRANDOMF(chunk->area.x_min, chunk->area.x_max),
                0,
                RSEEDRANDOMF(chunk->area.z_min, chunk->area.z_max)
            };

            pos.y = raycast_terrain(terrain, pos.x, pos.z).point.y;

            float start_height = 15.0;
            float dampen_height = 0.85;
            float start_cube_scale = 5.0;
            float dampen_cube_scale = 0.7;


            fractal->liquid_type = GetRandomValue(0, MAX_LQCONTENT_TYPES-1);
            switch(fractal->liquid_type) {
                
                case LQCONTENT_ENERGY:
                    fractal->berry_color = (Color){ 30, 255, 255, 255 };
                    break;

                case LQCONTENT_HEALTH:
                    fractal->berry_color = (Color){ 255, 80, 120, 255 };
                    break;

                    // ... More can be added later
            }

            Color start_color = (Color){
                fractal->berry_color.r * 0.2,
                fractal->berry_color.g * 0.2,
                fractal->berry_color.b * 0.2,
                255
            };
            Color end_color = fractal->berry_color;

            Vector3 rotation_weights = (Vector3){ 0.35, 0.2, 0.185 };
            int depth = 8;


            fractalgen_tree(
                    gst,
                    fractal,
                    depth,
                    rotation_weights,
                    start_height,
                    dampen_height,
                    start_cube_scale,
                    dampen_cube_scale,
                    start_color,
                    end_color
                    );


            fractal->scale = (Vector3){ 0.6, 0.4, 0.6 };
            fractal->transform = MatrixTranslate(pos.x, pos.y, pos.z);
            fractal->transform = MatrixMultiply(MatrixRotateY(RSEEDRANDOMF(-M_PI, M_PI)), fractal->transform);
            fractal->transform = MatrixMultiply(
                    MatrixScale(fractal->scale.x, fractal->scale.y, fractal->scale.z),
                    fractal->transform);
        }
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


struct chunk_t* find_chunk(struct state_t* gst, Vector3 position) {
    struct chunk_t* chunk = NULL;

    // Normalize coordinates to chunk positions.

    int c_x = position.x - gst->terrain.transform.m12;
    int c_z = position.z - gst->terrain.transform.m14;
    c_x /= (gst->terrain.chunk_size * gst->terrain.scaling);
    c_z /= (gst->terrain.chunk_size * gst->terrain.scaling);
    
    
    int chunks_inrow = (gst->terrain.heightmap.size / gst->terrain.chunk_size);

    c_x = CLAMP(c_x, 0, chunks_inrow-1);
    c_z = CLAMP(c_z, 0, chunks_inrow-1);


    size_t chunk_index = c_z * chunks_inrow + c_x;
    if(chunk_index >= gst->terrain.num_chunks) {
        chunk_index = gst->terrain.num_chunks-1;
    }

    chunk = &gst->terrain.chunks[chunk_index];
    if(chunk->index != chunk_index) {
        fprintf(stderr, "\033[35m(WARNING) '%s': Mismatch in found chunk index\033[0m\n",
                __func__);
    }

    return chunk;
}

void chunk_add_item(struct chunk_t* chunk, struct item_t* item) {
    if(!chunk) {
        return;
    }

    // If the chunk is not full just add it to next index.
    if(chunk->num_items+1 < MAX_ITEMS_PERCHUNK) {
        memmove(
                &chunk->items[chunk->num_items],
                item,
                sizeof *item
                );
        chunk->num_items++;
    }
    // The chunk may not have active item in all elements.
    // Check if there is one free to use.
    else {
        int item_added = 0;
        for(int i = 0; i < chunk->num_items; i++) {
            struct item_t* chunk_item = &chunk->items[i];
            if(!chunk_item->empty) {
                continue;
            }
            
            memmove(
                    &chunk->items[i],
                    item,
                    sizeof *item
                    );
            item_added = 1;
            break;
        }

        if(!item_added) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Chunk %li has too many items. Cant add new one\033[0m\n",
                    __func__, chunk->index);
        }
    }
}

void chunk_update_items(struct state_t* gst, struct chunk_t* chunk) {
    struct item_t* item = NULL;
    for(int i = 0; i < chunk->num_items; i++) {
        item = &chunk->items[i];
        if(item->empty || (item->inv_index >= 0)) {
            continue;
        }

        update_item(gst, item);
    }
}

void chunk_render_items(struct state_t* gst, struct chunk_t* chunk) {
    
    struct item_t* item = NULL;
    for(int i = 0; i < chunk->num_items; i++) {
        item = &chunk->items[i];
        if(item->empty || (item->inv_index >= 0)) {
            continue;
        }


        switch(item->type) {
            
            case ITEM_WEAPON_MODEL:
                render_weapon_model(gst, &item->weapon_model, item->transform);
                break;

            case ITEM_LQCONTAINER:
                render_lqcontainer(gst, &item->lqcontainer, item->transform);
                break;

            default:
                render_item(gst, item, item->transform);
                break;

        }
    }

}

struct enemy_t* chunk_add_enemy(
        struct state_t* gst,
        struct chunk_t* chunk,
        Vector3 position,
        int enemy_type,
        int enemy_mood,
        int adder_type
){
    struct enemy_t* entptr = NULL;

    if(!chunk) {
        chunk = find_chunk(gst, position);
    }

    uint16_t max_enemies = 0;
    if(adder_type == ENEMY_ADDED_BY_SYSTEM) {
        max_enemies = MAX_ENEMIES_PERCHUNK_SAFE;
    }
    else
    if(adder_type == ENEMY_ADDED_BY_CHUNK) {
        max_enemies = MAX_ENEMIES_PERCHUNK;
    }
    else {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid adder_type\033[0m\n",
                __func__);
        goto error;
    }


    if(chunk->num_enemies+1 >= max_enemies) {
        fprintf(stderr, 
                "\033[31m(ERROR) '%s': Cant add new enemy to chunk(%p) limit reached.\n",
                __func__, chunk);
        goto error;
    }

    entptr = &chunk->enemies[chunk->num_enemies];
    
    if(!create_enemy(gst, entptr, enemy_type, enemy_mood, position)) {
        entptr = NULL;
        goto error;
    }

    entptr->chunk = chunk;
    entptr->index_in_chunk = chunk->num_enemies;
    chunk->num_enemies++;

error:
    return entptr;
}

void chunk_relocate_enemy(struct enemy_t* enemy, struct chunk_t* new_chunk) {
    if(!enemy->chunk || !new_chunk) {
        return;
    }
    if(enemy->chunk->index == new_chunk->index) {
        fprintf(stderr, "\033[35m(WARNING) '%s': Current enemy chunk and new_chunk index match."
                "Nothing has been done.\033[0m\n", __func__);
        return;
    }

    if(new_chunk->num_enemies+1 >= MAX_ENEMIES_PERCHUNK) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to relocate enemy. new_chunk is full.\033[0m\n",
                __func__);
        return;
    }
   
    // Copy enemy to the next chunk.
    new_chunk->enemies[new_chunk->num_enemies] = *enemy;
    struct enemy_t* entptr = &new_chunk->enemies[new_chunk->num_enemies];

    printf("%s: %li -> %li\n", __func__, enemy->chunk->index, new_chunk->index);
    // This will remove the enemy from its own chunk.
    chunk_remove_enemy(enemy);

    // Update the enemy info.
    entptr->chunk = new_chunk;
    entptr->index_in_chunk = new_chunk->num_enemies;
   

    new_chunk->num_enemies++;
}

void chunk_remove_enemy(struct enemy_t* enemy) {
    if(enemy->chunk->num_enemies == 0) {
        return;
    }

    for(uint16_t i = enemy->index_in_chunk; i < enemy->chunk->num_enemies-1; i++) {
        
        struct enemy_t* to = &enemy->chunk->enemies[i];
        struct enemy_t* from = &enemy->chunk->enemies[i+1];

        *to = *from;

        if(to->index_in_chunk > 0) {
            to->index_in_chunk--;
        }
    }

    enemy->chunk->num_enemies--;
}

void chunk_update_enemies(struct state_t* gst, struct chunk_t* chunk) {
    for(uint16_t i = 0; i < chunk->num_enemies; i++) {
        update_enemy(gst, &chunk->enemies[i]);
    }
}

void chunk_render_enemies(struct state_t* gst, struct chunk_t* chunk) {
    for(uint16_t i = 0; i < chunk->num_enemies; i++) {
        render_enemy(gst, &chunk->enemies[i]);
    }
}

void chunk_update_fractals(struct state_t* gst, struct chunk_t* chunk) {
    for(int i = 0; i < chunk->num_fractals; i++) {
        struct fractal_t* fractal = &chunk->fractals[i];

        Vector3 base = (Vector3){ fractal->transform.m12, fractal->transform.m13, fractal->transform.m14 };
        if(Vector3Distance(base, gst->player.position) < 30
        && point_in_player_view(gst, &gst->player, base, 60.0)) {
         
            player_handle_action(gst, &gst->player, IACTION_HARVEST, IACTION_FOR_FRACTAL_TREE, fractal);
        }
    }
}


void chunk_render_fractals(struct state_t* gst, struct chunk_t* chunk, int render_pass) {


    for(int i = 0; i < chunk->num_fractals; i++) {
        struct fractal_t* fractal = &chunk->fractals[i];
 
        Vector3 base = (Vector3){ 
            fractal->transform.m12,
            fractal->transform.m13,
            fractal->transform.m14
        };  


        switch(render_pass) {
            case RENDERPASS_RESULT:
                fractal->material.shader = gst->shaders[FRACTAL_MODEL_SHADER];
                shader_setu_float(gst, FRACTAL_MODEL_SHADER, U_FRACTAL_BASE_Y, &base.y);
                break;
            
            case RENDERPASS_SHADOWS:
            case RENDERPASS_GBUFFER:
                fractal->material.shader = gst->shaders[FRACTAL_MODEL_GBUFFER_SHADER];
                shader_setu_float(gst, FRACTAL_MODEL_GBUFFER_SHADER, U_FRACTAL_BASE_Y, &base.y);
                break;
        }

        rlDisableBackfaceCulling();
        DrawMesh(
                fractal->mesh,
                fractal->material,
                fractal->transform
                );
        rlEnableBackfaceCulling();


        // Render berries only when player is very nearby.
        if(Vector3Distance(base, gst->player.position) > 500) {
            continue;
        }
       
        //printf("(%0.3f)%s: %i\n", gst->time, __func__, fractal->num_berries);

        shader_setu_color(gst, FRACTAL_BERRY_SHADER, U_BERRY_COLOR, &fractal->berry_color);

        for(int j = 0; j < fractal->num_berries; j++) {
            struct berry_t* berry = &fractal->berries[j];

            berry->position.x += cos(j*8 + gst->time * 2.0) * 0.003;
            berry->position.z += sin(j*8 + gst->time * 2.0) * 0.003;

            Matrix berry_m = MatrixTranslate(
                    berry->position.x * fractal->scale.x + base.x ,
                    berry->position.y * fractal->scale.y + base.y ,
                    berry->position.z * fractal->scale.z + base.z 
                    );

            berry_m = MatrixMultiply(MatrixScale(berry->level, berry->level, berry->level), berry_m);

            DrawMesh(
                    gst->fractal_berry_model.meshes[0],
                    gst->fractal_berry_model.materials[0],
                    berry_m
                    );
        }
    }
}

