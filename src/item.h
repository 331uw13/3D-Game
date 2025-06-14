#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <raylib.h>
#include <stddef.h>

#include "weapon_model.h"
#include "typedefs.h"


struct state_t;
struct chunk_t;
struct weapon_model_t;

#define MAX_ITEM_MODELS 16

#define ITEM_APPLE 0
#define ITEM_CONTAINER 1
// ...
#define MAX_ITEM_TYPES 2


#define ITEM_COMMON 0
#define ITEM_RARE 1
#define ITEM_SPECIAL 2
#define ITEM_MYTHICAL 3


#define ITEM_GROUND_YAXIS_PADDING 3.85


#define ITEM_MAX_NAME_SIZE 32
#define ITEM_MAX_DESC_SIZE 256






struct item_info_t {
    int item_type;

    char name[ITEM_MAX_NAME_SIZE];
    size_t name_size;

    char desc[ITEM_MAX_DESC_SIZE];
    size_t desc_size;
};


struct item_t {

    int count;
    uint16_t type;
    uint8_t empty;
    uint8_t rarity;

    float dst2player; // Distance to player.

    Model* modelptr;
    Matrix transform;
    
    struct item_info_t* info;

    Vector3 position; // Read only position.
    Vector3 velocity;
    float lifetime;

    int inv_index; // Index in inventory, set to '-1' if the item is not in inventory.

    int is_weapon_item;
    struct weapon_model_t weapon_model;
};



int load_item_model(
        struct state_t* gst, 
        int to_index,     // Index in 'gst->item_models' array.
        int tex_index,    // Index in 'gst->textures' array.
        uint8_t rarity,
        const char* model_filepath
        );

// TODO: Rename this. The name is little bit misleading.
struct item_t get_weapon_model_item(
            struct state_t* gst,
            int weapon_model_index
            );

Color get_item_rarity_color(struct item_t* item);


#define FIND_ITEM_CHUNK NULL

void spawn_item_type(
        struct state_t* gst,
        struct chunk_t* chunk, // If chunk is set to NULL, it has to be found by this function.
        Vector3 position,
        int type,
        int count);

void drop_item(
        struct state_t* gst,
        struct chunk_t* chunk,
        Vector3 position,
        struct item_t* item);


void update_item(struct state_t* gst, struct item_t* item);
void render_item(struct state_t* gst, struct item_t* item, Matrix transform);

#endif
