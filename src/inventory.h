#ifndef INVENTORY_H
#define INVENTORY_H

#include <raylib.h>


#define INV_SIZE 9

struct state_t;
struct player_t;

#include "item.h"

struct inventory_t {
    int open;

    float time_from_selected;
    struct item_t* items[INV_SIZE];
    int            selected_index; // Set to negative value if nothing selected.
    // ...

};


void render_inventory(struct state_t* gst, struct player_t* player);
void update_inventory(struct state_t* gst, struct player_t* player);
int inv_add_item(struct state_t* gst, struct player_t* player, struct item_t* item);

/*
void update_inventory(struct state_t* gst, struct player_t* player);
void render_inventory(struct state_t* gst, struct player_t* player);
void toggle_inventory(struct state_t* gst, struct player_t* player);

int inv_add_item(struct state_t* gst, struct player_t* player, struct item_t* item);

*/

#endif
