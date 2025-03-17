#include <stdio.h>
#include <math.h>


#include "gui/gui.h"
#include "inventory.h"
#include "state.h"
#include "util.h"


void update_inventory(struct state_t* gst, struct player_t* player) {
    if(!player->inventory.open) {
        return;
    }


}

void render_inventory(struct state_t* gst, struct player_t* player) {
    if(!player->inventory.open) {
        return;
    }

    Vector2 mouse = GetMousePosition();

    const Vector2 inv_size = (Vector2) {
        600,
        500
    };
    
    const Vector2 cell_size = (Vector2) {
        inv_size.x / INV_COLS,
        inv_size.y / INV_ROWS
    };
    
    const Vector2 inv_pos = (Vector2) {
        gst->scrn_w/2 - inv_size.x/2,
        gst->scrn_h/2 - inv_size.y/2,
    };

    Rectangle invrec_bg = (Rectangle) {
        inv_pos.x-10, inv_pos.y-10,
        inv_size.x+20, inv_size.y+20
    }; 
 
    DrawRectangle(0, 0, gst->scrn_w, gst->scrn_h, (Color){ 10, 10, 10, 80 });

    DrawRectangleV(inv_pos, inv_size, (Color){ 40, 40, 40, 200 });

    Color line_color = (Color) { 60, 60, 60, 200 };

    for(int y = 1; y < INV_ROWS; y++) {
        DrawLine(
                inv_pos.x,              y * cell_size.y + inv_pos.y,
                inv_pos.x + inv_size.x, y * cell_size.y + inv_pos.y,
                line_color
                );
    }
    
    for(int x = 1; x < INV_COLS; x++) {
        DrawLine(
                x * cell_size.x + inv_pos.x, inv_pos.y,
                x * cell_size.x + inv_pos.x, inv_pos.y + inv_size.y,
                line_color
                );
    }

    int mouse_rdown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);


    for(int y = 0; y < INV_ROWS; y++) {
        for(int x = 0; x < INV_COLS; x++) {

            const size_t index = y * INV_ROWS + x;
            struct item_t* item = player->inventory.items[index];
            Vector2 cell_pos = (Vector2) { inv_pos.x + x * cell_size.x, inv_pos.y + y * cell_size.y };
    
            int mouse_on = mouse_on_rect(cell_pos, cell_size);

            //printf("%p\n", item);
            

            if(item) {
                // Draw item texture on its place if its not the one being dragged.
                int drag = (item == player->inventory.item_drag);

                DrawTexturePro(
                        item->inv_tex,
                        (Rectangle) { // src
                            0, 0, item->inv_tex.width, item->inv_tex.height
                        },
                        (Rectangle) { // dest
                            cell_pos.x, cell_pos.y,
                            cell_size.x, cell_size.y
                        },
                        // No rotation.
                        (Vector2){0, 0}, 0, drag ? (Color){ 100, 100, 100, 255 } : WHITE
                        );
            
                if(drag) {
                    DrawTexturePro(
                            item->inv_tex,
                            (Rectangle) { // src
                                0, 0, item->inv_tex.width, item->inv_tex.height
                            },
                            (Rectangle) { // dest
                                mouse.x - cell_size.x/2, mouse.y - cell_size.y/2,
                                cell_size.x, cell_size.y
                            },
                            // No rotation.
                            (Vector2){0, 0}, 0, WHITE
                            );
                }
               
                Color rarity_color = item->rarity_color;

                if(item->rarity == ITEM_MYTHICAL) {
                    rainbow_palette(sin(gst->time), &rarity_color.r, &rarity_color.g, &rarity_color.b);
                }

                DrawRectangleLines(cell_pos.x, cell_pos.y, cell_size.x, cell_size.y, rarity_color);

            }

            if(mouse_on && mouse_rdown) {
                if(!player->inventory.item_drag) {
                    player->inventory.item_drag = item;
                    player->inventory.item_drag_from_index = index;
                }
            }

            if(mouse_on && !mouse_rdown && player->inventory.item_drag) {
                if(!player->inventory.items[index]) {
                    player->inventory.items[index] = player->inventory.item_drag;
                    player->inventory.items[player->inventory.item_drag_from_index] = NULL;
                }
                player->inventory.item_drag = NULL;
            }


            if(mouse_on && !mouse_rdown) {
                DrawRectangleLines(cell_pos.x, cell_pos.y, cell_size.x, cell_size.y, (Color){ 130, 130, 130, 255 });
            }
        }
    }


}



void toggle_inventory(struct state_t* gst, struct player_t* player) {

    if((player->inventory.open = !player->inventory.open)) {
        EnableCursor();
    }
    else {
        DisableCursor();
    }

}

int inv_add_item(struct state_t* gst, struct player_t* player, struct item_t* item) {
    int added = 0;

    for(size_t i = 0; i < INV_SIZE; i++) {
        if(player->inventory.items[i] == NULL) {
            player->inventory.items[i] = item;
            added = 1;
            break;
        }
    }

    if(!added) {
        fprintf(stderr, "\033[35m(WARNING) '%s': Inventory is full.\033[0m\n",
                __func__);
    }

    return added;
}



