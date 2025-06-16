#include <stdio.h>
#include <string.h>

#include "item.h"
#include "state/state.h"


// Texture index can be found from gst->textures array.
int load_item_model(struct state_t* gst, 
        int to_index,     // Index in gst->item_models array.
        int tex_index,    // Texture index in world.
        uint8_t rarity,
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

    gst->item_rarities[to_index] = rarity;

    Model* model = &gst->item_models[to_index];
    *model = LoadModel(model_filepath);
 
    model->materials[0] = LoadMaterialDefault();
    model->materials[0].shader = gst->shaders[DEFAULT_SHADER];
    model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[tex_index];

    result = 1;

error:
    return result;
}

struct item_t get_empty_item() {
    struct item_t item = (struct item_t) {
        .type = -1,
        .count = 1,
        .empty = 0,
        .rarity = 0,
        .dst2player = 999999,
        .modelptr = NULL,
        .info = NULL,
        .transform = MatrixIdentity(),
        .position = (Vector3){ 0 },
        .velocity = (Vector3){ 0 },
        .inv_index = -1,
        .is_special = 0,
        .is_weapon_item = 0,
        .is_lqcontainer_item = 0
    };
    return item;
}

struct item_t get_lqcontainer_item(struct state_t* gst) {
    struct item_t item = get_empty_item();

    item.modelptr = &gst->item_models[ITEM_LQCONTAINER];
    item.info = &gst->item_info[ITEM_LQCONTAINER];
    item.is_lqcontainer_item = 1;
    item.lqcontainer = (struct lqcontainer_t) {
        .level = 0,
        .capacity = 750,
        .content_type = LQCONTENT_EMPTY
    };
    item.is_special = 1;
    return item;
}

struct item_t get_weapon_model_item(struct state_t* gst, int weapon_model_index) {
    struct weapon_model_t* weapon_model = &gst->weapon_models[weapon_model_index];
    struct item_t item = get_empty_item();

    item.rarity = weapon_model->rarity;
    item.modelptr = &weapon_model->model;
    item.info = &gst->item_info[weapon_model->item_info_index];
    item.is_weapon_item = 1;
    item.weapon_model = *weapon_model;
    item.is_special = 1;
    
    return item;
}

Color get_item_rarity_color(struct item_t* item) {
    
    static const Color rarity_colors[] = {
        (Color){ 0x33, 0xD6, 0x49, 0xFF }, // COMMON
        (Color){ 0x33, 0xC8, 0xD6, 0xFF }, // RARE
        (Color){ 0xCF, 0x5E, 0xDB, 0xFF }, // SPECIAL
        (Color){ 0xF7, 0x39, 0x39, 0xFF }  // MYTHICAL
    };

    return rarity_colors[item->rarity];
}


void drop_item_type(struct state_t* gst,
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

    struct item_t item = get_empty_item();
    item.type = type;
    item.count = count;
    item.rarity = gst->item_rarities[type];
    item.modelptr = &gst->item_models[type];
    item.transform = MatrixTranslate(position.x, position.y, position.z);
    item.position = position;
    item.velocity = (Vector3){ 0.0, 1.0, 0.0 };
    item.info = &gst->item_info[type];
    /*
    struct item_t new_item = (struct item_t) {
        .type = type,
        .count = count,
        .rarity = gst->item_rarities[type],
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
    */

    chunk_add_item(chunk, &item);
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

    // Update item gravity.
    if(item->position.y > (tray.point.y + ITEM_GROUND_YAXIS_PADDING)) {
        item->position.y -= (item->velocity.y * 20.0) * gst->dt;
        item->transform = MatrixTranslate(item->position.x, item->position.y, item->position.z);
        item->velocity.y += (item->velocity.y * 3.0) * gst->dt;
    }
    else {
        item->velocity.y = 1.0;
    }

    item->position = (Vector3){ item->transform.m12, item->transform.m13, item->transform.m14 };

    // Raycast item if player is nearby so it can be picked up.
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
}

void render_item(struct state_t* gst, struct item_t* item, Matrix transform) {
    for(int i = 0; i < item->modelptr->meshCount; i++) {
        DrawMesh(
                item->modelptr->meshes[i],
                item->modelptr->materials[0],
                transform
                );
    }
}

void get_item_additional_info(struct item_t* item, char* buffer, size_t max_size, size_t* buffer_size_ptr) {
    
    if(item->is_weapon_item) {
        struct weapon_model_t* weapon_model = &item->weapon_model;

        const char* info = TextFormat(
                "Ammo: %0.1f / %0.1f\n"
                "Condition: <todo>\n"
                ,
                weapon_model->stats.lqmag.ammo_level,
                weapon_model->stats.lqmag.capacity
                );

        append_str(buffer, max_size, buffer_size_ptr, info);
    }
    else
    if(item->is_lqcontainer_item) {
        struct lqcontainer_t* lqcontainer = &item->lqcontainer;
    
        const char* info = TextFormat(
                "Level: %0.2f\n"
                "Capacity: %0.2f\n"
                "Content: <todo>\n"
                "Condition: <todo>\n"
                ,
                lqcontainer->level,
                lqcontainer->capacity
                );

        append_str(buffer, max_size, buffer_size_ptr, info);
    }


}


