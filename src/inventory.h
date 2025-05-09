#ifndef INVENTORY_H
#define INVENTORY_H

#include <raylib.h>


struct state_t;
struct player_t;

#include "item.h"



// 32 spaces for items.
#define INV_NUM_COLUMNS 8
#define INV_NUM_ROWS 4


struct inventory_t {
    int open;



};

void inventory_init(struct inventory_t* inv);
void inventory_render(struct state_t* gst, struct inventory_t* inv);



#endif
