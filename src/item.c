#include <stdio.h>
#include <string.h>

#include "state.h"
#include "item.h"
#include "util.h"
#include <raymath.h>


static const char* ITEM_NAMES[] = {
    "Apple\0",
    "<Unknown>\0"
};

static const int CONSUMABLE_ITEMS[] = {
    ITEM_APPLE,
    // ...
};

static const Color ITEM_RARITY_COLORS[] = {
    (Color) { 150, 100, 30, 255 }, // Common
    (Color) { 30, 150, 30, 255 },  // Rare
    (Color) { 30, 200, 200, 255 }, // Special
    (Color) { 240, 200, 30, 255 }, // Legendary
    (Color) { 0, 0, 0, 255 },      // Mythical (has rainbow effect)
};



static const int MAX_ITEMS_IN_WORLD[] = {
    15 /* ITEM_APPLE */    
    // ...
};

static const int ITEMS_SPAWN_CHANCE[] = { // 0% - 100%
    80 /* ITEM APPLE */
    // ...
};

static const float MAX_NATURAL_ITEMS_SPAWN_TIME[] = {
    5.0 /* ITEM APPLE */
    // ...
};


int load_item_model(struct state_t* gst, u32 item_type, const char* model_filepath, int texid) {
    int result = 0;

    if(!FileExists(model_filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Model path not found: \"%s\"\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    if(item_type >= MAX_ITEM_MODELS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'item_models' array doesnt have enough space.\033[0m\n",
                __func__);
        goto error;
    }

    SetTraceLogLevel(LOG_ALL);

    Model* model = &gst->item_models[item_type];
    *model = LoadModel(model_filepath);

    if(!IsModelValid(*model)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to load item model \"%s\"\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    model->materials[0] = LoadMaterialDefault();
    model->materials[0].shader = gst->shaders[DEFAULT_SHADER];
    model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[texid];



error:
    SetTraceLogLevel(LOG_NONE);
    return result;
}


void spawn_item(struct state_t* gst, u32 item_type, int inv_texid, int item_rarity, Vector3 position) {
    
    int num_this_type = 0;

    for(size_t i = 0; i < MAX_ALL_ITEMS; i++) {
        if(gst->items[i].enabled && (gst->items[i].type == item_type)) {
            num_this_type++;
        }
    }

    if(num_this_type >= MAX_ITEMS_IN_WORLD[item_type]) {
        //printf("'%s': Too many item '%i' in world\n", __func__, item_type);
        return;
    }

    size_t item_index = gst->num_items;
    if(item_index >= MAX_ALL_ITEMS) {
        size_t i = 0;
        for(; i < MAX_ALL_ITEMS; i++) {
            if(!gst->items[i].enabled) {
                item_index = i;
                break;
            }
        }
        if(i == MAX_ALL_ITEMS) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Max items reached.\033[0m\n",
                    __func__);
            return;
        }
    }


    struct item_t* item = &gst->items[item_index];

    item->modelptr = &gst->item_models[item_type];
    item->position = position;
    item->enabled = 1;
    item->lifetime = 0.0;
    item->type = item_type;
    item->transform = MatrixTranslate(position.x, position.y, position.z);
    item->dist_to_player = 0.0;
    item->name = ITEM_NAMES[item_type];
    item->index = item_index;
    item->inv_tex = gst->textures[inv_texid];
    item->pickedup = 0;
    item->rarity = item_rarity;
    item->rarity_color = ITEM_RARITY_COLORS[item_rarity];

    item->consumable = 0;

    const size_t num_consumables = sizeof(CONSUMABLE_ITEMS) / sizeof *CONSUMABLE_ITEMS;
    for(int i = 0; i < num_consumables; i++) {
        if(item_type == CONSUMABLE_ITEMS[i]) {
            item->consumable = 1;
            break;
        }
    }

    printf("'%s' Item %i\n", __func__, item_type);

    item->name_width = MeasureText(item->name, 20);

    gst->num_items++;
    if(gst->num_items >= MAX_ALL_ITEMS) {
        gst->num_items = MAX_ALL_ITEMS;
    }
}


void update_items(struct state_t* gst) {
    gst->player.item_in_crosshair = NULL;

    for(size_t i = 0; i < gst->num_items; i++) {
        struct item_t* item = &gst->items[i];
        if(!item->enabled) {
            continue;
        }

        item->lifetime += gst->dt;

        if(item->lifetime >= ITEM_MAX_LIFETIME) {
            item->enabled = 0;
            continue;
        }

        item->position.y += sin(gst->time*2.325+item->index) * 0.00425;
        Matrix rotation = MatrixRotateY(gst->time + item->index);
        item->transform = MatrixMultiply(rotation, MatrixTranslate(item->position.x, item->position.y, item->position.z));
        item->dist_to_player = Vector3Distance(gst->player.position, item->position);

        if(item->dist_to_player < ITEM_PICKUP_RADIUS) {
            // Cast ray from player eye to item, see if it hit.
 
            Ray ray2item = (Ray) {
                .position = gst->player.cam.position,
                .direction = Vector3Normalize(Vector3Subtract(gst->player.position, gst->player.cam.target))
            };

            RayCollision ray = GetRayCollisionSphere(
                    ray2item,
                    item->position,
                    ITEM_COLLISION_SPHERE_RAD
                    );

            if(!ray.hit) {
                continue;
            }
            
            // Confirm the ray hit was really correct.
            // for some reason 'GetRayCollsionSphere' may return 1 for the opposite direction..

            Vector3 forward = ray2item.direction;

            Vector3 P1 = (Vector3) { item->position.x, 0.0, item->position.z };
            Vector3 P2 = (Vector3) { gst->player.position.x, 0.0, gst->player.position.z };

            Vector3 D = Vector3Normalize(Vector3Subtract(P1, P2));
            float dot = Vector3DotProduct(D, forward);

            if(dot > 0.5) {
                continue;
            }

            gst->player.item_in_crosshair = item;
        }
    }
}

void render_items(struct state_t* gst) {
    for(size_t i = 0; i < gst->num_items; i++) {
        struct item_t* item = &gst->items[i];
        if(!item->enabled) {
            continue;
        }

        if(item->dist_to_player > RENDER_DISTANCE) {
            continue;
        }
       


        DrawMesh(
                item->modelptr->meshes[0],
                item->modelptr->materials[0],
                item->transform
                );
    }
}


static Vector3 get_good_item_spawn_pos(struct state_t* gst) {
    Vector3 pos = (Vector3) { 0, 0, 0 };

    const int max_attemps = 100;
    int attemps = 0;

    while(attemps < max_attemps) {
        pos = (Vector3) {
            gst->player.position.x + RSEEDRANDOMF(-NATURAL_ITEM_SPAWN_RADIUS, NATURAL_ITEM_SPAWN_RADIUS),
            0,
            gst->player.position.x + RSEEDRANDOMF(-NATURAL_ITEM_SPAWN_RADIUS, NATURAL_ITEM_SPAWN_RADIUS)
        };

        RayCollision ray = raycast_terrain(&gst->terrain, pos.x, pos.z);
        int in_water = (ray.point.y < gst->terrain.water_ylevel);

        pos.y = ray.point.y + 5.0;

        if(!in_water && (Vector3Distance(pos, gst->player.position) > 50.0)) {
            break;
        }
        attemps++;
    }

    // TODO: handle if not good position found.

    return pos;
}

void update_natural_item_spawns(struct state_t* gst) {
    
    for(int item_type = 0; item_type < NUM_NATURAL_ITEMS; item_type++) {
        gst->natural_item_spawn_timers[item_type] += gst->dt;
        if(gst->natural_item_spawn_timers[item_type] >= MAX_NATURAL_ITEMS_SPAWN_TIME[item_type]) {
            gst->natural_item_spawn_timers[item_type] = 0.0;

            if(GetRandomValue(0, 100) < ITEMS_SPAWN_CHANCE[item_type]) {
                spawn_item(gst, item_type, APPLE_INV_TEXID, ITEM_COMMON, get_good_item_spawn_pos(gst));
            }
        }
    }

}

void setup_natural_item_spawn_settings(struct state_t* gst) {
    gst->natural_item_spawn_timers[ITEM_APPLE] = 0.0;
}



