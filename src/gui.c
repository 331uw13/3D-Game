#include <stdio.h>
#include <math.h>

#include "gui.h"
#include "state/state.h"
#include "util.h"

#include "state/state_free.h"   // Included for ability to reload shaders.
#include "state/state_setup.h"  //


int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size) {
    Vector2 mouse = GetMousePosition();

    return (
            (mouse.x > rect_pos.x) && (mouse.x < (rect_pos.x+rect_size.x)) &&
            (mouse.y > rect_pos.y) && (mouse.y < (rect_pos.y+rect_size.y))
            );

}


int gui_button(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position
){
    int clicked = 0;
    Vector2 measure = MeasureTextEx(gst->font, text, font_size, FONT_SPACING);

    Rectangle rect = (Rectangle) {
        position.x - EXTRA_OFF_W,   position.y - EXTRA_OFF_H,
        measure.x  + EXTRA_OFF_W*2,  measure.y + EXTRA_OFF_H*1.5
    };

    int mouse_on = mouse_on_rect((Vector2){ rect.x, rect.y }, (Vector2){ rect.width, rect.height });
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, mouse_on ? BUTTON_BG_FOCUS_COLOR : BUTTON_BG_COLOR);
    DrawTextEx(gst->font, text, position, font_size, FONT_SPACING, TEXT_COLOR);


    if(mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        clicked = 1;
    }


    return clicked;
}

int gui_slider_float(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position,
        float width,
        float* value_ptr,
        float min_value,
        float max_value
){
    Vector2 measure = MeasureTextEx(gst->font, text, font_size, FONT_SPACING);

    Rectangle rect = (Rectangle) {
        position.x - EXTRA_OFF_W, position.y - EXTRA_OFF_H,
        width + EXTRA_OFF_W*2, measure.y + EXTRA_OFF_H*1.5
    };

    int mouse_on = mouse_on_rect((Vector2){ rect.x, rect.y }, (Vector2){ rect.width, rect.height });

    DrawRectangle(rect.x, rect.y, rect.width, rect.height, BUTTON_BG_COLOR);

    float bar_width = map(*value_ptr, min_value, max_value, 0, rect.width);
    DrawRectangle(rect.x, rect.y, bar_width, rect.height, 
            mouse_on ? (Color){ 50, 90, 70, 200 } : (Color){ 40, 73, 85, 200 });


    DrawTextEx(gst->font, text, position, font_size, FONT_SPACING, TEXT_COLOR);

    int isactive = (mouse_on && IsMouseButtonDown(MOUSE_LEFT_BUTTON));

    if(isactive) {
        Vector2 mouse = GetMousePosition();
        float value = map(mouse.x, rect.x, rect.x+rect.width,  min_value, max_value);
        *value_ptr = value;
    }

    return isactive;
}

int gui_checkbox(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position,
        int* ptr
){
    int clicked = 0;

    Rectangle rect = (Rectangle) {
        position.x - EXTRA_OFF_W, position.y - EXTRA_OFF_H,
        40.0, 40.0
    };

    int mouse_on = mouse_on_rect((Vector2){ rect.x, rect.y }, (Vector2){ rect.width, rect.height });
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, mouse_on ? BUTTON_BG_FOCUS_COLOR : BUTTON_BG_COLOR);

    DrawTextEx(gst->font, text, (Vector2){position.x+15, position.y}, font_size, FONT_SPACING, TEXT_COLOR);

    if(*ptr) {
        DrawRectangle(rect.x+3, rect.y+3, rect.width-6, rect.height-6, (Color){ 50, 180, 200, 200 });
    }

    if(mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *ptr = !*ptr;
        clicked = 1;
    }

    return clicked;
}

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
    DrawTextEx(gst->font, text, position, font_size, FONT_SPACING, TEXT_COLOR);
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


