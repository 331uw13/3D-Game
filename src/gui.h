#ifndef GAME_GUI_H
#define GAME_GUI_H

#include <raylib.h>


struct state_t;

int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size);


int gui_button(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position
        );


void gui_render_respawn_screen(struct state_t* gst);
void gui_render_menu_screen(struct state_t* gst);
void gui_render_powerup_shop(struct state_t* gst);
void gui_render_devmenu(struct state_t* gst);

#endif
