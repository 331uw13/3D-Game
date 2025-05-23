#ifndef GAME_GUI_H
#define GAME_GUI_H

#include <raylib.h>
#include "gui_render.h"



// Extra space for text.
#define EXTRA_OFF_H 10.0   // How much extra space for Height
#define EXTRA_OFF_W 30.0   // How much extra space for Width


#define GUICFG_MAX_SAVE_SLOTS 16

#define GUI_CONTAINER_MAX_NAME_SIZE 32

struct gui_container_t {
    Rectangle rect;
    int auto_size;
    char name[GUI_CONTAINER_MAX_NAME_SIZE];
    int padding_x;
    int padding_y;
    int open;
};

struct guicfg_t {
    float font_size;
    int next_y;
    int next_x;
    int y_space;  // Space between components vertical.
    int x_space;  // Space between components horizontal.
    int prev_component_width;
    int prev_component_height;
    int prev_component_x;
    int prev_component_y;

    int in_container;
    struct gui_container_t container;

    // Empty space for text inside buttons and stuff.
    int padding_x;
    int padding_y;
    

    // First slot is reserved for gui_next_x/y__previous(...) calls.
    // [n][0]: (is x or y saved already?)
    // [n][1]: next_x
    // [n][2]: next_y
    int saved[GUICFG_MAX_SAVE_SLOTS][3];
    
    Color text_color;
    Color container_text_color;

    Color component_bg_color__unfocus;
    Color component_bg_color__focus;

    Color component_fg_color__unfocus;
    Color component_fg_color__focus; 

    Color component_highlight__unfocus;
    Color component_highlight__focus;

    Color component_color__enabled;
    Color component_color__sliderv_focus;
    Color component_color__sliderv_unfocus;
    Color component_color_slider_value;

};

#define SLIDER_DEF_WIDTH 400



int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size);
void gui_load_default_cfg(struct guicfg_t* guicfg);



// ------- Gui Control -------

void gui_begin(struct state_t* gst);
void gui_end(struct state_t* gst);


#define GUI_CONTAINER_AUTO_SIZE -1, -1
void gui_container(struct state_t* gst, struct guicfg_t* guicfg, const char* name, int width, int height, int* open);
void gui_end_container(struct state_t* gst, struct guicfg_t* guicfg);


// Next X Position is set to previous component X position PLUS previous component width.
// Saves old next X.
void gui_next_x__previous(struct guicfg_t* guicfg);

// Next Y Position is set to previous component Y position.
// Saves old next Y.
void gui_next_y__previous(struct guicfg_t* guicfg);

// Save current next_x and next_y into 'slot'.
void gui_save_pos(struct guicfg_t* guicfg, int slot);


// Sets 'next_x' to saved X position from saved 'slot'(index to guicfg.saved array)
void gui_next_x__saved(struct guicfg_t* guicfg, int slot);

// Sets 'next_y' to saved Y position from saved 'slot'(index to guicfg.saved array)
void gui_next_y__saved(struct guicfg_t* guicfg, int slot);



// ------- Gui Components -------

void gui_text(struct state_t* gst, struct guicfg_t* guicfg, const char* text);

// Returns positive number when clicked.
int gui_button(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        const char* text
        );

// Returns positive number when clicked.
int gui_checkbox(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        const char* text,
        int* ptr
        );

// Returns positive number when active.
// ID Can be any unique number.
int gui_sliderF(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        int id, 
        const char* text,
        float  width,
        float* value,
        float  min_value,
        float  max_value
        );

// Returns positive number when active.
// ID Can be any unique number.
int gui_sliderI(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        int id, 
        const char* text,
        float  width,
        int* value,
        int  min_value,
        int  max_value
        );





// ----- EXTRA -----

// Returns positive number when active.
int gui_colorpicker(
        struct state_t* gst,
        const char* text,
        Vector2 position,
        Vector2 size,
        Color* color
        );



#endif
