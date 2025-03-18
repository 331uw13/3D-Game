#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <raylib.h>
#include <stddef.h>

#include "typedefs.h"


#define MAX_ALL_ITEMS 32
#define MAX_ITEM_MODELS 1

#define MAX_ITEM_NAME_SIZE 32
#define ITEM_MAX_LIFETIME 60.0 // (in seconds)
#define ITEM_PICKUP_RADIUS 15.0

#define ITEM_COLLISION_SPHERE_RAD 2.0
#define NATURAL_ITEM_SPAWN_RADIUS 500.0

// IMPORTANT NOTE: All natural items should be first.
#define ITEM_APPLE 0
#define NUM_NATURAL_ITEMS 1

#define MAX_ITEM_TYPES 1
// ...


#define APPLE_HEALTH_INCREASE 25


#define ITEM_COMMON 0
#define ITEM_RARE 1
#define ITEM_SPECIAL 2
#define ITEM_LEGENDARY 3
#define ITEM_MYTHICAL 4


struct state_t;

struct item_t {
    int     enabled;
    int     pickedup;

    int     rarity;
    int     type;
    int     consumable; // Can it be eaten?

    Model*  modelptr;
    Vector3 position;
    float   lifetime;
    size_t  index;  // Index in state 'items' array.
    Matrix  transform;
    float   dist_to_player;

    Texture inv_tex; // Texture for inventory.
    Color rarity_color;
        
    char* name; // Null terminated.
    int   name_width; // Width for rendering.
};


int load_item_model(
        struct state_t* gst,
        u32 item_type,
        const char* model_filepath,
        int inv_texindex
        );


void spawn_item(struct state_t* gst, u32 item_type, int inv_texid, int item_rarity, Vector3 position);

void update_items(struct state_t* gst);
void render_items(struct state_t* gst);

void update_natural_item_spawns(struct state_t* gst);
void setup_natural_item_spawn_settings(struct state_t* gst);


#endif
