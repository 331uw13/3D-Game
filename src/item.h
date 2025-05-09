#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <raylib.h>
#include <stddef.h>

#include "typedefs.h"

// IMPORTANT NOTE: 
// Remember to add new item name
// into 'src/item.c' if adding new item.



struct state_t;
struct chunk_t;


#define MAX_ITEM_MODELS 16

#define ITEM_APPLE 0
// ...
#define MAX_ITEM_TYPES 1


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


#define ITEM_STATE_DROPPED 0
#define ITEM_STATE_PICKEDUP 1

struct item_t {

    int state;
    int type;
    int count;
    int empty;

    float dst2player; // Distance to player.

    Model* modelptr;
    Matrix transform;
    
    struct item_info_t* info;

    Vector3 position; // Read only position.
    Vector3 velocity;
    float lifetime;
};



int load_item_model(struct state_t* gst, 
        int to_index,     // Index in 'gst->item_models' array.
        int tex_index,    // Index in 'gst->textures' array.
        const char* model_filepath);


void spawn_item(struct state_t* gst,
        struct chunk_t* chunk, // If chunk is set to NULL, it has to be found by this function.
        Vector3 position,
        int type,
        int count);

void update_item(struct state_t* gst, struct item_t* item);


#endif
