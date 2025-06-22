#ifndef INVENTORY_H
#define INVENTORY_H

#include <raylib.h>


struct state_t;
struct player_t;

#include "items/item.h"
#include "light.h"


// 32 spaces for items.
#define INV_NUM_COLUMNS 8
#define INV_NUM_ROWS 4
#define INV_SIZE (INV_NUM_COLUMNS * INV_NUM_ROWS)


struct inventory_t {
    int open;
    
    struct light_t* light;
    struct item_t* selected_item;
    struct item_t* hovered_item;

    struct item_t items[INV_SIZE];
    
    uint8_t item_drag;
    float mouse_down_timer;
};

void inventory_init(struct inventory_t* inv);
void inventory_render(struct state_t* gst, struct inventory_t* inv);

#define INV_INDEX_NEXT_FREE -1
void inventory_move_item(struct state_t* gst, struct inventory_t* inv, struct item_t* item, int index);

void inventory_open(struct state_t* gst, struct inventory_t* inv);
void inventory_close(struct state_t* gst, struct inventory_t* inv);


#endif
