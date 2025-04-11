#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <raylib.h>
#include <stddef.h>

#include "typedefs.h"


#define MAX_ALL_ITEMS 32
#define MAX_ITEM_MODELS 2

#define MAX_ITEM_NAME_SIZE 32
#define ITEM_MAX_LIFETIME 60.0 // (in seconds)
#define ITEM_PICKUP_RADIUS 15.0
#define ITEM_HOVER_LEVEL 3.0

#define ITEM_COLLISION_SPHERE_RAD 2.0
#define NATURAL_ITEM_SPAWN_RADIUS 500.0

// IMPORTANT NOTE: All natural items should be first.
#define ITEM_APPLE 0
#define NUM_NATURAL_ITEMS 1

// Other items: (dropped by enemies or can be crafted)
#define ITEM_METALPIECE 1 // Dropped by enemies.
#define MAX_ITEM_TYPES 2
// ...


#define ITEM_COMMON 0
#define ITEM_RARE 1
#define ITEM_SPECIAL 2
#define ITEM_LEGENDARY 3
#define ITEM_MYTHICAL 4

#define ITEM_DROP_CHANCE_MIN 0
#define ITEM_DROP_CHANCE_MAX 1000


struct state_t;

struct item_t {
    int     enabled;
    int     pickedup; // TODO: remove this.

    int     can_be_dropped;

    int     rarity;
    u32     type;

    int     consumable; 
    float   health_boost_when_used;
    float   armor_fix_value; 


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


    // TODO: Description
};


int load_item_model(
        struct state_t* gst,
        u32 item_type,
        const char* model_filepath,
        int inv_texindex
        );


void use_consumable_item(struct state_t* gst, struct item_t* item);
void spawn_item(struct state_t* gst, u32 item_type, Vector3 position);

void update_items(struct state_t* gst);
void render_items(struct state_t* gst);

void update_natural_item_spawns(struct state_t* gst);
void setup_natural_item_spawn_settings(struct state_t* gst);

// Returns value from 0 to 1000
int get_item_drop_chance(u32 item_type);


#endif
