#include "gui.h"
#include "state.h"


#define EXTRA_OFF_H 10.0    // How much extra space for Height
#define EXTRA_OFF_W 30.0   // How much extra space for Width

#define TEXT_COLOR        (Color) { 160, 176, 180, 255 }
#define BUTTON_BG_COLOR   (Color) { 30, 40, 50, 200 }
#define FOCUS_COLOR       (Color) { 50, 150, 170, 200 }
#define UNFOCUS_COLOR     (Color) { 50, 70, 80, 200 }


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

    DrawRectangle(rect.x, rect.y, rect.width, rect.height, BUTTON_BG_COLOR);
    DrawTextEx(gst->font, text, position, font_size, FONT_SPACING, TEXT_COLOR);

    int mouse_on = mouse_on_rect((Vector2){ rect.x, rect.y }, (Vector2){ rect.width, rect.height });


    DrawRectangleLines(rect.x, rect.y, rect.width, rect.height,
            mouse_on ? FOCUS_COLOR : UNFOCUS_COLOR);

    if(mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        clicked = 1;
    }


    return clicked;
}


void gui_render_respawn_screen(struct state_t* gst) {
    
    DrawRectangle(0, 0, gst->scrn_w, gst->scrn_h, (Color){ 10, 10, 10, 150 });
    DrawTextEx(gst->font, 
            TextFormat("Enemies killed: %i", gst->player.kills),
            (Vector2){ 30, 200 },
            20.0,
            FONT_SPACING,
            TEXT_COLOR
            );


    if(gui_button(gst, "Respawn", 30.0, (Vector2){100, gst->scrn_h/2})) {
        player_respawn(gst, &gst->player);
    }

    if(gui_button(gst, "Exit", 30.0, (Vector2){100, gst->scrn_h/2 + 70})) {
        gst->running = 0;
    }


}

void gui_render_menu_screen(struct state_t* gst) {
    
    DrawRectangle(0, 0, gst->scrn_w, gst->scrn_h, (Color){ 10, 10, 10, 150 });
    DrawTextEx(gst->font, 
            TextFormat("Enemies killed: %i", gst->player.kills),
            (Vector2){ 30, 200 },
            20.0,
            FONT_SPACING,
            TEXT_COLOR
            );
    
    if(gui_button(gst, "Exit", 30.0, (Vector2){100, gst->scrn_h/2})) {
        gst->running = 0;
    }

    if(gui_button(gst, "Toggle Fullscreen", 20.0, (Vector2){100, gst->scrn_h/2 + 80})) {
        
        if(!IsWindowFullscreen()) {
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        }
        ToggleFullscreen();
        
        if(!IsWindowFullscreen()) {
            SetWindowSize(DEF_SCRN_W, DEF_SCRN_H);
        }
    }
    



}


