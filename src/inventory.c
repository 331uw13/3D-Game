#include <stdio.h>
#include <math.h>


#include "gui.h"
#include "inventory.h"
#include "state/state.h"
#include "util.h"


void render_inventory(struct state_t* gst, struct player_t* player) {
    
    float slot_space = 5.0;

    Vector2 slot_size = (Vector2) { 40.0, 40.0 };
    Vector2 inv_pos = (Vector2) { 5.0, 200.0 };
    Vector2 inv_size = (Vector2) { slot_size.x+10, (slot_size.y+slot_space)*INV_SIZE + slot_space };

    DrawRectangleV(inv_pos, inv_size, (Color){ 30, 30, 30, 200 });

    for(int i = 0; i < INV_SIZE; i++) {

        Vector2 slot_pos = (Vector2){ inv_pos.x+slot_space,  inv_pos.y+slot_space + (i*(slot_size.y+slot_space)) };
        DrawRectangleV(slot_pos, slot_size, (Color){ 70, 70, 70, 200 });

        int iselected = (i == player->inventory.selected_index);

        if(iselected) {
            DrawRectangleLinesEx((Rectangle){ slot_pos.x-2, slot_pos.y-2, slot_size.x+4, slot_size.y+4 }, 2.0, GREEN);
        
        }

        struct item_t* item = player->inventory.items[i];
        if(!item) {
            continue;
        }

        if((i > 0) && iselected) {
            DrawTextEx(gst->font, "<E>: Drop item\n<MouseRight>: Use item",
                    (Vector2){ inv_pos.x, inv_pos.y-35 }, 13.0, 1.0,
                    ColorLerp((Color){ 200, 200, 200, 255 }, (Color){200, 200, 200, 0},
                            player->inventory.time_from_selected));

        }

        DrawTexturePro(
                item->inv_tex,
                (Rectangle) { // src
                    0, 0, item->inv_tex.width, item->inv_tex.height
                },
                (Rectangle) { // dest
                    slot_pos.x, slot_pos.y,
                    slot_size.x, slot_size.y
                },
                (Vector2){0, 0}, 0, // No rotation.
                WHITE
                );

        


    }
}

void update_inventory(struct state_t* gst, struct player_t* player) {
    
    float mouse_wheel = GetMouseWheelMove();
    
    if(player->inventory.time_from_selected < 1.0) {
        player->inventory.time_from_selected += gst->dt*0.75;
    }

    if(mouse_wheel > 0) {
        player->inventory.selected_index--;
        player->inventory.time_from_selected = 0.0;
    }
    else
    if(mouse_wheel < 0) {
        player->inventory.selected_index++;
        player->inventory.time_from_selected = 0.0;
    }
    player->inventory.selected_index = CLAMP(player->inventory.selected_index, 0, INV_SIZE-1);


    size_t selected_i = player->inventory.selected_index;
    struct item_t* selected = player->inventory.items[selected_i];
    if(selected) {
        if(IsKeyPressed(KEY_E) && selected->can_be_dropped) {
            spawn_item(gst, selected->type, 
                    (Vector3){player->position.x, player->position.y-3.0, player->position.z });
            player->inventory.items[selected_i] = NULL;
        }
        else
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (selected->consumable)) {
            use_consumable_item(gst, selected);
            player->inventory.items[selected_i] = NULL;
        }
    }
}


int inv_add_item(struct state_t* gst, struct player_t* player, struct item_t* item) {
    int result = 0;
    for(size_t i = 0; i < INV_SIZE; i++) {
        if(!player->inventory.items[i]) {
            player->inventory.items[i] = item;
            result = 1;
            break;
        }

    }

    return result;
}

/*
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
        gst->scrn_w/2 - inv_size.x/2 + 200,
        gst->scrn_h/2 - inv_size.y/2,
    };

    Rectangle invrec_bg = (Rectangle) {
        inv_pos.x-10, inv_pos.y-10,
        inv_size.x+20, inv_size.y+20
    }; 
    
    Vector2 descbox_size = (Vector2) { 300, inv_size.y };
    Vector2 descbox_pos = (Vector2) { inv_pos.x - 350, inv_pos.y };

 
    DrawRectangle(0, 0, gst->scrn_w, gst->scrn_h, (Color){ 10, 10, 10, 80 });
    DrawRectangleV(
            (Vector2){ descbox_pos.x-30, descbox_pos.y-30 },
            (Vector2){
                    descbox_size.x+inv_size.x+120,
                    inv_size.y+60

                }, (Color){ 20, 20, 20, 100 });
    
    DrawRectangleV(inv_pos, inv_size, (Color){ 40, 40, 40, 200 });

    Color line_color = (Color) { 60, 60, 60, 200 };

    for(int y = 1; y < INV_ROWS; y++) {
        DrawLine(
                inv_pos.x,              y * (cell_size.y+1) + inv_pos.y,
                inv_pos.x + inv_size.x, y * (1+cell_size.y) + inv_pos.y,
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

            if(mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                player->inventory.selected_item_index = index;
            }

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
                    // The item is being dragged. draw its inventory texture on mouse position.
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
               

                // Draw rarity color rect
                Color rarity_color = item->rarity_color;
                if(item->rarity == ITEM_MYTHICAL) {
                    rainbow_palette(sin(gst->time), &rarity_color.r, &rarity_color.g, &rarity_color.b);
                }
                DrawRectangleLines(cell_pos.x, cell_pos.y, cell_size.x, cell_size.y, rarity_color);
            
                // Indicate selected item.
                if(item->index == player->inventory.selected_item_index) {
                    int blink = 100 + (sin(gst->time*8.0)*0.5+0.5) * 100;
                    DrawRectangleLines(cell_pos.x+4, cell_pos.y+4, cell_size.x-8, cell_size.y-8,
                            (Color){ blink, blink, blink, 180 });
                }
            }



            if(mouse_on && mouse_rdown && !player->inventory.item_drag) {
                // Item drag begin.
                player->inventory.item_drag = item;
                player->inventory.item_drag_from_index = index;
                player->inventory.selected_item_index = -1;
            }

            if(mouse_on && !mouse_rdown && player->inventory.item_drag) {
                // Item drag end.
                if(!player->inventory.items[index]) {
                    player->inventory.items[index] = player->inventory.item_drag;
                    player->inventory.items[player->inventory.item_drag_from_index] = NULL;
                    player->inventory.items[index]->index = index;
                }

                player->inventory.selected_item_index = -1;
                player->inventory.item_drag = NULL;
            }


            if(mouse_on && !mouse_rdown) {
                DrawRectangleLines(cell_pos.x, cell_pos.y, cell_size.x, cell_size.y, (Color){ 130, 130, 130, 255 });
            }
        }
    }


    // Draw selected item description and action buttons.
 
    DrawRectangleV(descbox_pos, descbox_size, (Color){ 20, 20, 20, 200 });

   
    if((player->inventory.selected_item_index < 0) 
    || (player->inventory.selected_item_index >= INV_SIZE)) {
        return;
    }

    struct item_t* selected_item = player->inventory.items[player->inventory.selected_item_index];
    if(selected_item) {
        Vector2 name_pos = (Vector2) { descbox_pos.x+10, descbox_pos.y+10 };
        DrawTextEx(gst->font, selected_item->name, name_pos, 
                20.0, FONT_SPACING, (Color){ 200, 200, 200, 255 });


        float btn_x = descbox_pos.x+50.0;
        float btn_y = descbox_pos.y+50.0;
        const float btn_y_inc = 40.0;

        if(selected_item->can_fix_armor) {
            if(gui_button(gst, "Fix armor", 15.0, (Vector2){ btn_x, btn_y })) {
                player->armor += selected_item->armor_fix_value;
                player->armor = CLAMP(player->armor, 0, player->max_armor);
                player->inventory.items[selected_item->index] = NULL;
                player->inventory.selected_item_index = -1;
            }
            btn_y += btn_y_inc;
        }
        else 
        if(selected_item->consumable) {
            if(gui_button(gst, "Eat", 15.0, (Vector2){ btn_x, btn_y })) {
                player_heal(gst, &gst->player, selected_item->health_boost_when_eaten);
                player->inventory.items[selected_item->index] = NULL;
                printf("%li\n", selected_item->index);
                player->inventory.selected_item_index = -1;
            }
            btn_y += btn_y_inc;
        }

        if(gui_button(gst, "Drop", 15.0, (Vector2){ btn_x, btn_y })) {
            spawn_item(gst, selected_item->type, 
                    (Vector3) {
                        gst->player.cam.position.x,
                        gst->player.cam.position.y-3.0,
                        gst->player.cam.position.z
                    });
            player->inventory.items[selected_item->index] = NULL;
            player->inventory.selected_item_index = -1;
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
            player->inventory.items[i]->index = i;
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
*/


