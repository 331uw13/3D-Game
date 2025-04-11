#ifndef GAME_GUI_H
#define GAME_GUI_H

#include <raylib.h>
#include "gui_render.h"



// Extra space for text.
#define EXTRA_OFF_H 10.0   // How much extra space for Height
#define EXTRA_OFF_W 30.0   // How much extra space for Width


// TODO: Make color palette system for gui components

#define TEXT_COLOR        (Color) { 160, 176, 180, 255 }
#define BUTTON_BG_COLOR   (Color) { 30, 40, 50, 200 }
#define BUTTON_BG_FOCUS_COLOR (Color) { 40, 60, 70, 200 }
#define FOCUS_COLOR       (Color) { 50, 150, 170, 200 }
#define UNFOCUS_COLOR     (Color) { 50, 70, 80, 200 }



int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size);


// Gui Components.

// Returns positive number when clicked.
int gui_button(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position
        );

// Returns positive number when active.
int gui_slider_float(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position,
        float width,
        float* value,
        float min_value,
        float max_value
        );

// Returns positive number when clicked.
int gui_checkbox(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position,
        int* ptr
        );

// Returns positive number when active.
int gui_colorpicker(
        struct state_t* gst,
        const char* text,
        Vector2 position,
        Vector2 size,
        Color* color
        );



#endif
