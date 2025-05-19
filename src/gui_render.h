#ifndef GUI_RENDER_H
#define GUI_RENDER_H

struct state_t;
struct inventory_t;

// Render and update all type of gui.


void gui_render_respawn_screen(struct state_t* gst);
void gui_render_menu_screen(struct state_t* gst);
void gui_render_powerup_shop(struct state_t* gst);
void gui_render_devmenu(struct state_t* gst);

void render_item_info(struct state_t* gst);

void gui_render_inventory_controls(struct state_t* gst, struct inventory_t* inv);



#endif
