#ifndef GUI_RENDER_H
#define GUI_RENDER_H

struct state_t;

// Render and update all type of gui.


void gui_render_respawn_screen(struct state_t* gst);
void gui_render_menu_screen(struct state_t* gst);
void gui_render_powerup_shop(struct state_t* gst);
void gui_render_devmenu(struct state_t* gst);






#endif
