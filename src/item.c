#include <stdio.h>
#include <string.h>

#include "item.h"
#include "state/state.h"


// Texture index can be found from gst->textures array.
int load_item_model(struct state_t* gst, 
        int to_index,     // Index in gst->item_models array.
        int tex_index,    // Texture index in world.
        const char* model_filepath
){
    int result = 0;

    if(to_index >= MAX_ITEM_MODELS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'to_index' is out of bounds. Resize the item models array?\033[0m\n",
                __func__);
        goto error;
    }

    if(!FileExists(model_filepath)) {
    
        fprintf(stderr, "\033[31m(ERROR) '%s': Model file '%s' Not found\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    Model* model = &gst->item_models[to_index];
    *model = LoadModel(model_filepath);
 
    model->materials[0] = LoadMaterialDefault();
    model->materials[0].shader = gst->shaders[DEFAULT_SHADER];
    model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[tex_index];

    result = 1;

error:
    return result;
}


struct item_t get_weapon_model_item(
            struct state_t* gst,
            int weapon_model_index
){
    struct weapon_model_t* weapon_model = &gst->weapon_models[weapon_model_index];

    struct item_t item = (struct item_t) {
        .type = -1,
        .count = 1,
        .empty = 0,
        .dst2player = 10000,
        .modelptr = &weapon_model->model,
        .transform = MatrixIdentity(),
        .info = &gst->item_info[weapon_model->item_info_index],
        .position = (Vector3){ 0 },
        .velocity = (Vector3){ 0 },
        .lifetime = 0.0,
        .inv_index = -1,
        .is_weapon_item = 1,
        .weapon_model = *weapon_model
    };
    
    return item;
}


void spawn_item_type(struct state_t* gst,
        struct chunk_t* chunk, // If chunk is set to NULL, it has to be found by this function.
        Vector3 position,
        int type,
        int count
){
    if(!chunk) {
        chunk = find_chunk(gst, position);
    }


    // Make sure item doesnt spawn below ground level.
    RayCollision tray = raycast_terrain(&gst->terrain, position.x, position.z);
    float item_min_y = (tray.point.y + ITEM_GROUND_YAXIS_PADDING);
    if(position.y < item_min_y) {
        position.y = item_min_y;
    }


    struct item_t new_item = (struct item_t) {
        .type = type,
        .count = count,
        .empty = 0,
        .lifetime = 0.0,
        .modelptr = &gst->item_models[type],
        .transform = MatrixTranslate(position.x, position.y, position.z),
        .position = position,
        .velocity = (Vector3) { 0.0, 1.0, 0.0 },
        .info = &gst->item_info[type],
        .inv_index = -1,
        .is_weapon_item = 0,
        .weapon_model = (struct weapon_model_t) { 0 }
    };

    chunk_add_item(chunk, &new_item);
}

void drop_item(
        struct state_t* gst,
        struct chunk_t* chunk,
        Vector3 position,
        struct item_t* item
){

    if(!chunk) {
        chunk = find_chunk(gst, position);
    }

    // Make sure item doesnt spawn below ground level.
    RayCollision tray = raycast_terrain(&gst->terrain, position.x, position.z);
    float item_min_y = (tray.point.y + ITEM_GROUND_YAXIS_PADDING);
    if(position.y < item_min_y) {
        position.y = item_min_y;
    }

    item->inv_index = -1;
    item->position = position;
    item->transform = MatrixTranslate(position.x, position.y, position.z);
    item->velocity = (Vector3){ 0.0, 1.0, 0.0 };

    chunk_add_item(chunk, item);
}


void update_item(struct state_t* gst, struct item_t* item) {
    if(item->inv_index >= 0) {
        return;
    }

    item->dst2player = Vector3Distance(item->position, gst->player.position);
    RayCollision tray = raycast_terrain(&gst->terrain, item->position.x, item->position.z);

    if(item->position.y > (tray.point.y + ITEM_GROUND_YAXIS_PADDING)) {
        item->position.y -= (item->velocity.y * 20.0) * gst->dt;
        item->transform = MatrixTranslate(item->position.x, item->position.y, item->position.z);
        item->velocity.y += (item->velocity.y * 3.0) * gst->dt;
    }
    else {
        item->velocity.y = 1.0;
    }



    // Raycast item if player is nearby.
    if(item->dst2player < 20) {
        Vector3 raydir = Vector3Subtract(gst->player.cam.position, gst->player.cam.target);

        Ray ray = (Ray) {
            .position = gst->player.cam.position,
            .direction = Vector3Normalize(raydir)
        };

        RayCollision rayres = GetRayCollisionSphere(ray, item->position, 7.5);
    
        if(rayres.hit) {
            gst->crosshair_item_info = item->info;

            if(gst->player.wants_to_pickup_item) {
                printf("Pickedup '%s'\n", item->info->name);
                inventory_move_item(gst, &gst->player.inventory, item, INV_INDEX_NEXT_FREE);
            }
        }
    }

    if(item->inv_index >= 0 && item->is_weapon_item) {
        item->weapon_model.light->enabled = 0;
    }
}


