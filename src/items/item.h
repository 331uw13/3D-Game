#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <raylib.h>
#include <stddef.h>

#include "weapon_model.h"
#include "lqcontainer.h"
#include "typedefs.h"


struct state_t;
struct chunk_t;
struct weapon_model_t;

#define MAX_ITEM_MODELS 16

#define ITEM_APPLE 0
#define ITEM_LQCONTAINER 1
#define ITEM_WEAPON_MODEL 2 // NOTE: Weapon model doesnt have its model in gst->item_models array.
// ...
#define MAX_ITEM_TYPES 3


#define ITEM_COMMON 1
#define ITEM_RARE 2
#define ITEM_SPECIAL 3
#define ITEM_MYTHICAL 4


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
    int8_t rarity;

    float dst2player; // Distance to player.

    Model* modelptr;
    Matrix transform;
    Matrix last_pview_transform; // Latest transform used when rendered item in players hand.
    struct item_info_t* info;

    Vector3 position; // "Read only" position. Update the transform instead.
    Vector3 velocity;
    float lifetime;

    int inv_index; // Index in inventory, set to '-1' if the item is not in inventory.

    double rotation;
    uint8_t is_special;


    struct weapon_model_t weapon_model;
    struct lqcontainer_t lqcontainer;
};


#define MAX_ITEM_COMBINE_TYPES 16
#define CANT_COMBINE_ITEMS -2
#define ITEM_COMBINE_RES_BY_HANDLER -1 // Combined result is handled 
                                       // by some function in 'src/items/item_combine.c'
                                       // _not_ 'combine_items()' fucntion.


#define ICINFO_TYPE 0
#define ICINFO_RESULT 1

// Item combine info can be found in 'state.item_combine_data' array by 'item.type' as index.
struct item_combine_info_t {
    // [0] (ICINFO_TYPE) = the other item type.
    // [1] (ICINFO_RESULT) = the result type when items are combined.
   
    int16_t types[MAX_ITEM_COMBINE_TYPES][2];
    int8_t  num_types;
    
    void    (*combine_callbacks[MAX_ITEM_COMBINE_TYPES])
            (struct state_t*, struct item_t*, struct item_t*);

};


int load_item_model(
        struct state_t* gst, 
        int to_index,     // Index in 'gst->item_models' array.
        int tex_index,    // Index in 'gst->textures' array.
        uint8_t rarity,
        const char* model_filepath
        );


// IMPORTANT NOTE: 'get_empty_item()' will have its modelptr set to NULL.
struct item_t get_empty_item();
struct item_t get_weapon_model_item(struct state_t* gst, int weapon_model_index);
struct item_t get_lqcontainer_item(struct state_t* gst);


Color get_item_rarity_color(struct item_t* item);

// Used in 'gui_render.c' gui_render_inventory_controls()
void get_item_additional_info(struct item_t* item, char* buffer, size_t max_size, size_t* buffer_size_ptr);

// Returns item type or 'ITEM_COMBINE_RES_BY_HANDLER'
// 'CANT_COMBINE_ITEMS' is returned if error happened.
// 'found_info_index' is set if not NULL,
//   it is at which point in item_combine_info.types, the type_B was found.
int get_item_combine_result(struct state_t* gst, int type_A, int type_B, int* found_info_index);

// This function may use 'item_combine_info.callbacks[found_info_index]'
// if 'get_item_combine_result()' returns 'ITEM_COMBINE_RES_BY_HANDLER'.
void combine_items(struct state_t* gst, struct item_t* item_A, struct item_t* item_B);

#define FIND_ITEM_CHUNK NULL

// IMPORTANT NOTE: 'drop_item_type()' cant drop special items.
// Special items are for example: weapons and liquid containers(lqcontainer).
// Because of all of the special items doesnt have model in state.item_models array.
void drop_item_type(
        struct state_t* gst,
        struct chunk_t* chunk,
        Vector3 position,
        int type,
        int count);

void drop_item(
        struct state_t* gst,
        struct chunk_t* chunk,
        Vector3 position,
        struct item_t* item);

void update_item(struct state_t* gst, struct item_t* item);

// NOTE: Use special item's own rendering function.
// This is for rendering very simple items only.
void render_item(struct state_t* gst, struct item_t* item, Matrix transform);


#endif
