#include <stdio.h>
#include <math.h>

#include "gui.h"
#include "state/state.h"
#include "util.h"

#include "state/state_free.h"   // Included for ability to reload shaders.
#include "state/state_setup.h"  //

#define GUICFG_SAVED_FLG 0
#define GUICFG_SAVED_X 1
#define GUICFG_SAVED_Y 2
#define GUICFG_NOT_SAVED (-0xFFFF)

#define GUICFG_X_SAVED_FLG (1<<0)
#define GUICFG_Y_SAVED_FLG (1<<1)

#define ROUNDNESS 0.25

static int g_active_slider_id = -1;


static Color get_state_bg_color(struct guicfg_t* guicfg, int mouse_on) {
Color color = guicfg->component_bg_color__unfocus;
    if(mouse_on) {
        color = guicfg->component_bg_color__focus;
    }

    return color;
}
static Color get_state_fg_color(struct guicfg_t* guicfg, int mouse_on) {
    Color color = guicfg->component_fg_color__unfocus;
    if(mouse_on) {
        color = guicfg->component_fg_color__focus;
    }

    return color;
}
static Color get_state_highlight_color(struct guicfg_t* guicfg, int mouse_on) {
    Color color = guicfg->component_highlight__unfocus;
    if(mouse_on) {
        color = guicfg->component_highlight__focus;
    }

    return color;
}

static void gui_component_added(struct guicfg_t* guicfg, Vector2 pos, Vector2 size) {
    guicfg->prev_component_width = size.x + guicfg->x_space;
    guicfg->prev_component_height = size.y + guicfg->y_space;
    guicfg->prev_component_x = pos.x;
    guicfg->prev_component_y = pos.y;
    guicfg->next_y += size.y + guicfg->y_space;

    // Want to keep track of max width for container.
    // if its in auto size mode.
    if(guicfg->in_container && guicfg->container.auto_size) {
    
        Rectangle* rect = &guicfg->container.rect;
        rect->height = guicfg->next_y - rect->y;

        int added_width_x = guicfg->prev_component_width + (pos.x - rect->x);
        if(rect->width < added_width_x) {
            rect->width = added_width_x;
        }

    }
}

int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size) {
    Vector2 mouse = GetMousePosition();

    return (
            (mouse.x > rect_pos.x) && (mouse.x < (rect_pos.x+rect_size.x)) &&
            (mouse.y > rect_pos.y) && (mouse.y < (rect_pos.y+rect_size.y))
            );

}


void gui_load_default_cfg(struct guicfg_t* guicfg) {
    guicfg->next_y = 10;
    guicfg->next_x = 10;
    guicfg->y_space = 5;
    guicfg->x_space = 10;
    guicfg->font_size = 15;
    guicfg->prev_component_width = 0;
    guicfg->prev_component_height = 0;
    guicfg->prev_component_x = 0;
    guicfg->prev_component_y = 0;
    
    guicfg->padding_x = 20;
    guicfg->padding_y = 8;

    for(int i = 0; i < GUICFG_MAX_SAVE_SLOTS; i++) {
        guicfg->saved[i][GUICFG_SAVED_FLG] = 0;
        guicfg->saved[i][GUICFG_SAVED_X] = GUICFG_NOT_SAVED;
        guicfg->saved[i][GUICFG_SAVED_Y] = GUICFG_NOT_SAVED;
    }

    // Setup container.
    guicfg->in_container = 0;
    guicfg->container.open = 0;
    guicfg->container.auto_size = 0;
    guicfg->container.rect = (Rectangle) { 0 };
    
    guicfg->container.padding_x = 8;
    guicfg->container.padding_y = 12;
    

    // Default colors.

    guicfg->component_bg_color__unfocus = (Color){ 0x1F, 0x24, 0x24, 0xC8 };
    guicfg->component_bg_color__focus   = (Color){ 0x26, 0x30, 0x30, 0xC8 };
    
    guicfg->component_fg_color__unfocus = (Color){ 0x76, 0x80, 0x80, 0xFF };
    guicfg->component_fg_color__focus   = (Color){ 0x80, 0xAB, 0xAB, 0xFF };
    
    guicfg->component_highlight__unfocus = (Color){ 0x45, 0x7D, 0x7D, 0xFF };
    guicfg->component_highlight__focus   = (Color){ 0x1E, 0xC9, 0xC9, 0xFF };

    guicfg->component_color__enabled     = (Color){ 0x46, 0xB8, 0x9F, 0xC8 };
    guicfg->component_color__sliderv_focus   = (Color){ 0x37, 0x94, 0x96, 0xC8 };
    guicfg->component_color__sliderv_unfocus = (Color){ 0x57, 0x77, 0x78, 0xC8 };
    guicfg->component_color_slider_value = (Color){ 0x54, 0x6A, 0x6B, 0xC8 };

    guicfg->container_text_color = (Color){ 0x6D, 0x6E, 0x6E, 0xFF };
    guicfg->text_color = (Color){ 0x9A, 0xA5, 0xA6, 0xFF };
}

void gui_begin(struct state_t* gst) {
    BeginTextureMode(gst->gui_render_target);
    ClearBackground((Color){ 0, 0, 0, 0 });
}


void gui_end(struct state_t* gst) {
    EndTextureMode();

    DrawTexturePro(
            gst->gui_render_target.texture,
            (Rectangle) {
                0, 0,
                gst->gui_render_target.texture.width,
                -gst->gui_render_target.texture.height,
            },
            (Rectangle) {
                0, 0,
                gst->screen_size.x,
                -gst->screen_size.y,
            },
            (Vector2){0}, 0, WHITE
            );

}


void gui_container(struct state_t* gst, struct guicfg_t* guicfg, const char* name, int width, int height, int* open) {
    guicfg->in_container = 1;
    guicfg->container.auto_size = (width < 0 && height < 0);
    guicfg->container.rect = (Rectangle) {
        guicfg->next_x,
        guicfg->next_y,
        width,
        height
    };
    Vector2 text_size = MeasureTextEx(gst->font, name, guicfg->font_size, FONT_SPACING);
    
    Vector2 checkbox_pos = (Vector2) {
        guicfg->next_x,
        guicfg->next_y + guicfg->padding_y / 2
    };

    Vector2 checkbox_size = (Vector2) {
        text_size.y, text_size.y
    };


    Vector2 text_pos = (Vector2) {
        guicfg->next_x + checkbox_size.x + guicfg->padding_x,
        guicfg->next_y + guicfg->padding_y / 2 + 2.0
    };
    
    DrawTextEx(gst->font, name, text_pos, guicfg->font_size/1.25, FONT_SPACING, guicfg->container_text_color);

    int mouse_on = mouse_on_rect(checkbox_pos, checkbox_size);


    DrawRectangleRoundedLinesEx(
            (Rectangle) {
                checkbox_pos.x, checkbox_pos.y,
                checkbox_size.x, checkbox_size.y
            },
            0.3,
            8,
            2.0,
            get_state_highlight_color(guicfg, mouse_on)
            );

    const float indicator_font_size = guicfg->font_size / 1.5;
    const char* indicator_text = *open ? "v" : "^";
    Vector2 indicator_size = MeasureTextEx(gst->font, indicator_text, indicator_font_size, FONT_SPACING);

    Vector2 indicator_pos = (Vector2) {
        checkbox_pos.x + indicator_size.x/2 - 1,
        checkbox_pos.y + indicator_size.y/2 - 1
    };

    DrawTextEx(gst->font, indicator_text, indicator_pos, indicator_font_size, FONT_SPACING, guicfg->container_text_color);

    if(mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *open = !*open;
    }
    
    guicfg->container.open = *open;
    guicfg->next_y += text_size.y;

    if(!*open) {
        guicfg->container.rect.width = text_size.x + indicator_size.x + guicfg->padding_x;
        guicfg->container.rect.height = text_size.y + guicfg->padding_y;
    }
    else {
        guicfg->next_y += guicfg->container.padding_y;
    }

}

#define MIN(x, y) ((x < y) ? y : x)

void gui_end_container(struct state_t* gst, struct guicfg_t* guicfg) {
    if(guicfg->container.auto_size) {
        guicfg->container.rect.width += 1;
        guicfg->container.rect.height += 1;
    }


    EndTextureMode();

    // Draw container.

    DrawRectangleRounded(
            (Rectangle) {
                guicfg->container.rect.x - guicfg->container.padding_x,
                guicfg->container.rect.y,
                MIN(guicfg->container.rect.width, 300) + guicfg->container.padding_x * 2,
                guicfg->container.rect.height
            },
            guicfg->container.open ? 0.08 : 0.32,
            8,
            (Color){ 17, 17, 20, 190 }
            );

    if(!guicfg->container.open) {
    }

    guicfg->container.rect.width = 0;
    guicfg->container.rect.height = 0;

    BeginTextureMode(gst->gui_render_target);
    guicfg->next_y += guicfg->container.padding_y;
    guicfg->in_container = 0;
}


void gui_next_x__previous(struct guicfg_t* guicfg) {
    if(guicfg->in_container && guicfg->container.auto_size) { return; } 
    if(!(guicfg->saved[0][GUICFG_SAVED_FLG] & GUICFG_X_SAVED_FLG)) {
        guicfg->saved[0][GUICFG_SAVED_FLG] |= GUICFG_X_SAVED_FLG;
        guicfg->saved[0][GUICFG_SAVED_X] = guicfg->next_x;
    }
    guicfg->next_x = guicfg->prev_component_x + guicfg->prev_component_width;
}

void gui_next_y__previous(struct guicfg_t* guicfg) {
    if(guicfg->in_container && guicfg->container.auto_size) { return; } 
    if(!(guicfg->saved[0][GUICFG_SAVED_FLG] & GUICFG_Y_SAVED_FLG)) {
        guicfg->saved[0][GUICFG_SAVED_FLG] |= GUICFG_Y_SAVED_FLG;
        guicfg->saved[0][GUICFG_SAVED_Y] = guicfg->next_y - guicfg->prev_component_height;
    }
    guicfg->next_y = guicfg->prev_component_y;
}

void gui_save_pos(struct guicfg_t* guicfg, int slot) {
    if(guicfg->in_container && guicfg->container.auto_size) { return; } 
    if(slot == 0) {
        fprintf(stderr, "\033[31m(ERROR): '%s': first index is reserved\033[0m\n",
                __func__);
        return;
    }

    guicfg->saved[slot][GUICFG_SAVED_FLG] = 0;
    guicfg->saved[slot][GUICFG_SAVED_X] = guicfg->next_x;
    guicfg->saved[slot][GUICFG_SAVED_Y] = guicfg->next_y;
}


void gui_next_x__saved(struct guicfg_t* guicfg, int slot) {
    if(guicfg->in_container && guicfg->container.auto_size) { return; } 
    if(slot >= GUICFG_MAX_SAVE_SLOTS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'slot' out of bounds.\033[0m\n",
                __func__);
        return;
    }

    guicfg->next_x = guicfg->saved[slot][GUICFG_SAVED_X];

    // Tell gui config that the X can be set again.
    guicfg->saved[slot][GUICFG_SAVED_FLG] &= ~GUICFG_X_SAVED_FLG;
}


void gui_next_y__saved(struct guicfg_t* guicfg, int slot) {
    if(guicfg->in_container && guicfg->container.auto_size) { return; } 
    if(slot >= GUICFG_MAX_SAVE_SLOTS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'slot' out of bounds.\033[0m\n",
                __func__);
        return;
    }

    guicfg->next_y = guicfg->saved[slot][GUICFG_SAVED_Y];

    // Tell gui config that the Y can be set again.
    guicfg->saved[slot][GUICFG_SAVED_FLG] &= ~GUICFG_Y_SAVED_FLG;
}



void gui_text(struct state_t* gst, struct guicfg_t* guicfg, const char* text) {
    if(guicfg->in_container && !guicfg->container.open) {
        return;
    }

    Vector2 pos = (Vector2) {
        guicfg->next_x, guicfg->next_y
    };

    Vector2 size = MeasureTextEx(gst->font, text, guicfg->font_size, FONT_SPACING);

    DrawTextEx(gst->font, text, pos, guicfg->font_size, FONT_SPACING, guicfg->text_color);
    gui_component_added(guicfg, pos, size);
}


// Returns positive number when clicked.
int gui_button(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        const char* text
){
    if(guicfg->in_container && !guicfg->container.open) {
        return 0;
    }
    Vector2 text_size = MeasureTextEx(gst->font, text, guicfg->font_size, FONT_SPACING);
    Vector2 pos = (Vector2) { guicfg->next_x, guicfg->next_y + 2 };
    Vector2 size = (Vector2) {
        text_size.x + guicfg->padding_x,
        text_size.y + guicfg->padding_y
    };

    int mouse_on = mouse_on_rect(pos, size) && (g_active_slider_id < 0);
    int is_active = mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Button highlight.
    DrawRectangleRoundedLinesEx(
            (Rectangle) {
                pos.x-1,
                pos.y-1,
                size.x+2,
                size.y+2,
            },
            ROUNDNESS,
            8,
            2.0,
            get_state_highlight_color(guicfg, mouse_on));

    // Button background.
    DrawRectangleV(pos, size, get_state_bg_color(guicfg, mouse_on));
   

    Vector2 text_pos = (Vector2) {
        pos.x + (size.x/2.0 - text_size.x/2.0),
        pos.y + guicfg->padding_y / 2.0 + 1.0
    };

    // Button text.
    DrawTextEx(
            gst->font,
            text,
            text_pos,
            guicfg->font_size,
            FONT_SPACING,
            get_state_fg_color(guicfg, mouse_on)
            );

    size.y += 4; // Highlight size.
    gui_component_added(guicfg, pos, size);

    return is_active;
}


// Returns positive number when clicked.
int gui_checkbox(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        const char* text,
        int* ptr
){
    if(guicfg->in_container && !guicfg->container.open) {
        return 0;
    }
    Vector2 text_size = MeasureTextEx(gst->font, text, guicfg->font_size, FONT_SPACING);
    Vector2 pos = (Vector2) { guicfg->next_x, guicfg->next_y };
    Vector2 size = (Vector2) {
        text_size.x + guicfg->padding_x + 2,
        text_size.y + guicfg->padding_y + 2
    };

    int mouse_on = mouse_on_rect(pos, size) && (g_active_slider_id < 0);
    int is_active = mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Checkbox background.
    DrawRectangleRounded(
            (Rectangle) {
                pos.x, pos.y,
                size.x+size.y, size.y
            },
            ROUNDNESS,
            8,
            get_state_bg_color(guicfg, mouse_on)
            );

    // Checkbox
    DrawRectangleRoundedLinesEx(
            (Rectangle) {
                pos.x+4,
                pos.y+4,
                size.y-8,
                size.y-8
            },
            1.0, // Roundness.
            8,   // Segments.
            1.5, // Line width.
            get_state_fg_color(guicfg, mouse_on)
            );

    if(*ptr) {
        DrawRectangleRounded(
                (Rectangle) {
                    pos.x+4,
                    pos.y+4,
                    size.y-8,
                    size.y-8
                },
                1.0,
                8,
                guicfg->component_color__enabled
                );
    }

    Vector2 text_pos = (Vector2) {
        pos.x + size.y + 4.0,
        pos.y + (guicfg->padding_y+2)/2.0+1.0
    };

    // Checkbox text.
    DrawTextEx(
            gst->font,
            text,
            text_pos,
            guicfg->font_size,
            FONT_SPACING,
            get_state_fg_color(guicfg, mouse_on)
            );

    if(is_active) {
        *ptr = !*ptr;
    }

    size.x += size.y; // Checkbox size.
    gui_component_added(guicfg, pos, size);

    return is_active;
}


// Returns positive number when active.
int gui_sliderF(
        struct state_t* gst,
        struct guicfg_t* guicfg,
        int id,
        const char* text,
        float  width,
        float* value,
        float  min_value,
        float  max_value
){
    if(guicfg->in_container && !guicfg->container.open) {
        return 0;
    }

    Vector2 text_size = MeasureTextEx(gst->font, text, guicfg->font_size, FONT_SPACING);
    Vector2 pos = (Vector2) { guicfg->next_x, guicfg->next_y };
    Vector2 size = (Vector2) {
        width,
        text_size.y + guicfg->padding_y + 2
    };
    
    Vector2 mouse = GetMousePosition();
    int mouse_on = mouse_on_rect(pos, size);
    int is_active = mouse_on && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    if(is_active && (g_active_slider_id < 0)) {
        g_active_slider_id = id;
    }

    int is_changing = (g_active_slider_id == id);
    int state = is_changing || (mouse_on && g_active_slider_id < 0);

    if(is_changing) {
        // Map mouse position to value.
        *value = map(
                mouse.x, 
                pos.x,
                pos.x+size.x,
                min_value,
                max_value
                );

        *value = CLAMP(*value, min_value, max_value);
    }

    if(!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        g_active_slider_id = -1;
    }



    // Slider background
    DrawRectangleRounded(
            (Rectangle) {
                pos.x, pos.y,
                size.x, size.y
            },
            ROUNDNESS,
            8,
            get_state_bg_color(guicfg, state)
            );

    // Slider value indicator.
    Vector2 indicator_pos = (Vector2) {
        map(*value, min_value, max_value, pos.x, pos.x+size.x),
        pos.y
    };

    const float indicator_size = 8.0;
    DrawRectangleRounded(
            (Rectangle) {
                indicator_pos.x - (indicator_size/2.0),
                indicator_pos.y - 2,
                indicator_size,
                size.y+4
            },
            0.65,
            8,
            state
            ? guicfg->component_color__sliderv_focus
            : guicfg->component_color__sliderv_unfocus
            );


    Vector2 text_pos = (Vector2) {
        pos.x + guicfg->padding_x-2.0,
        pos.y + guicfg->padding_y/2.0+2.0
    };

    // Slider text.
    DrawTextEx(
            gst->font,
            text,
            text_pos,
            guicfg->font_size,
            FONT_SPACING,
            get_state_fg_color(guicfg, state)
            );

    
    // Slider value text.

    const float value_font_size = guicfg->font_size / 1.25;
    const char* value_text = TextFormat("%0.2f", *value);
    Vector2 value_text_size = MeasureTextEx(gst->font, value_text, value_font_size, FONT_SPACING);
    
    Vector2 value_text_pos = (Vector2) {
        (pos.x + width) - value_text_size.x - guicfg->padding_x,
        text_pos.y + 1.25
    };

    DrawTextEx(
            gst->font,
            value_text,
            value_text_pos,
            value_font_size,
            FONT_SPACING,
            guicfg->component_color_slider_value
            );

    gui_component_added(guicfg, pos, size);
    return is_active;
}








// ----- EXTRA -----

int gui_colorpicker(
        struct state_t* gst,
        const char* text,
        Vector2 position,
        Vector2 size,
        Color* color
){
    float font_size = 15;
    int isactive = 0;
    Vector2 origin = position;
    Vector2 text_size = MeasureTextEx(gst->font, text, font_size, FONT_SPACING);

    float size_extra = (text_size.x > size.x) ? (text_size.x - size.x) : 0.0;
    DrawRectangleV((Vector2){position.x-5, position.y-5 }, (Vector2){size.x+size_extra+10, size.y+text_size.y+20},
            (Color){ 20, 20, 20, 200 });
    DrawTextEx(gst->font, text, position, font_size, FONT_SPACING, WHITE);
    DrawLine(position.x, position.y+text_size.y, position.x+text_size.x, position.y+text_size.y, 
            (Color){ 120, 120, 120, 255});
    position.y += text_size.y+10;


    DrawTexturePro(gst->colorpick_tex,
            (Rectangle){ 0, 0, gst->colorpick_tex.width, -gst->colorpick_tex.height },
            (Rectangle){ position.x, position.y, size.x, size.y },
            (Vector2){0}, 0.0, WHITE);


    // Map mouse position to texture size.
    Vector2 mouse = GetMousePosition();
    Vector2 img_size = (Vector2){ gst->colorpick_img.width, gst->colorpick_img.height };

    int mx_on_tex = map(mouse.x, position.x, position.x+size.x, 0, img_size.x);
    int my_on_tex = map(mouse.y, position.y, position.y+size.y, 0, img_size.y);
    mx_on_tex = CLAMP(mx_on_tex, 0, img_size.x);
    my_on_tex = CLAMP(my_on_tex, 0, img_size.y);


    // Color can be then taken from image data with the coordinates just mapped.
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && mouse_on_rect(position, size)) {
        size_t pixel_index = (img_size.y-my_on_tex) * img_size.x + mx_on_tex;
        size_t max_pixel = img_size.x * img_size.y;
        pixel_index = (pixel_index > max_pixel) ? max_pixel : pixel_index;

        Color* img_pixels = (Color*)gst->colorpick_img.data;
        Color pixel_color = img_pixels[pixel_index];
  
        *color = pixel_color;
        DrawCircleLinesV(mouse, 8.0, WHITE);
        isactive = 1;
    }

    //printf("(%li): %i, %i, %i\n", pixel_index, pixel_color.r, pixel_color.g, pixel_color.b);
    DrawCircle(origin.x-15, origin.y+8.0, 10.0, *color);

    return isactive;
}


