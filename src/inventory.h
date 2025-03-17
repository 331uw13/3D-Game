#ifndef INVENTORY_H
#define INVENTORY_H

#include <raylib.h>

#define INV_ROWS 6
#define INV_COLS 6

#define INV_SIZE (INV_ROWS * INV_COLS)

struct state_t;
struct player_t;

#include "item.h"

struct inventory_t {
    int open;

    struct item_t* items[INV_SIZE];
    struct item_t* item_drag;
    int            item_drag_from_index;
    int            selected_index;
    // ...

};


void update_inventory(struct state_t* gst, struct player_t* player);
void render_inventory(struct state_t* gst, struct player_t* player);
void toggle_inventory(struct state_t* gst, struct player_t* player);

int inv_add_item(struct state_t* gst, struct player_t* player, struct item_t* item);


#endif
