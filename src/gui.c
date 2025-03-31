#include <stdio.h>
#include <math.h>

#include "gui.h"
#include "state.h"
#include "util.h"


#define EXTRA_OFF_H 10.0    // How much extra space for Height
#define EXTRA_OFF_W 30.0   // How much extra space for Width

#define TEXT_COLOR        (Color) { 160, 176, 180, 255 }
#define BUTTON_BG_COLOR   (Color) { 30, 40, 50, 200 }
#define BUTTON_BG_FOCUS_COLOR (Color) { 40, 60, 70, 200 }
#define FOCUS_COLOR       (Color) { 50, 150, 170, 200 }
#define UNFOCUS_COLOR     (Color) { 50, 70, 80, 200 }


int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size) {
    Vector2 mouse = GetMousePosition();

    return (
            (mouse.x > rect_pos.x) && (mouse.x < (rect_pos.x+rect_size.x)) &&
            (mouse.y > rect_pos.y) && (mouse.y < (rect_pos.y+rect_size.y))
            );

}


static int gui_button_ext(
        struct state_t* gst,
        const char* text,
        float font_size,
        Vector2 position,
        Color bg_color, // Background color
        Color fg_color, // Foreground color (text)
        Color focus_color,
        Color unfocus_color
){
    int clicked = 0;
    Vector2 measure = MeasureTextEx(gst->font, text, font_size, FONT_SPACING);

    Rectangle rect = (Rectangle) {
        position.x - EXTRA_OFF_W,   position.y - EXTRA_OFF_H,
        measure.x  + EXTRA_OFF_W*2,  measure.y + EXTRA_OFF_H*1.5
    };

    int mouse_on = mouse_on_rect((Vector2){ rect.x, rect.y }, (Vector2){ rect.width, rect.height });
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, mouse_on ? focus_color : unfocus_color);
    DrawTextEx(gst->font, text, position, font_size, FONT_SPACING, fg_color);

    


    if(mouse_on && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        clicked = 1;
    }


    return clicked;
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


static void render_num_kills(struct state_t* gst) {
    DrawTextEx(gst->font, 
           "Kills:",
           (Vector2){ 30, 50.0 },
           20.0,
           FONT_SPACING,
           (Color){ 180, 230, 240, 255 }
           );


    float kills_text_y = 80.0;
    for(size_t i = 0; i < MAX_ENEMY_TYPES; i++) {
        DrawTextEx(gst->font, 
                TextFormat("Enemy LVL%i: %i", i, gst->player.kills[i]),
                (Vector2){ 50, kills_text_y },
                15.0,
                FONT_SPACING,
                TEXT_COLOR
                );
        kills_text_y += 23.0;
    }
}


void gui_render_respawn_screen(struct state_t* gst) {
    
    
    render_num_kills(gst);

    if(gui_button(gst, "Respawn", 30.0, (Vector2){100, gst->res_y/2})) {
        player_respawn(gst, &gst->player);
    }

    if(gui_button(gst, "Exit", 30.0, (Vector2){100, gst->res_y/2 + 70})) {
        gst->running = 0;
    }
}


void gui_render_menu_screen(struct state_t* gst) {
    
    DrawRectangle(0, 0, gst->res_x, gst->res_y, (Color){ 10, 10, 10, 150 });
    render_num_kills(gst);

    if(gui_button(gst, "Exit", 30.0, (Vector2){100, gst->res_y/2})) {
        gst->running = 0;
    }

    Vector2 btn_pos = (Vector2){ 100, gst->res_y/2+80 };

    gui_checkbox(gst, "SSAO Enabled", 20.0, btn_pos, &gst->ssao_enabled);
    btn_pos.y += 50.0;

    gui_slider_float(gst, "Render Distance", 20.0, btn_pos, 530,
            &gst->menu_slider_render_dist_v, MIN_RENDERDIST, MAX_RENDERDIST);
    if(gui_button(gst, "Apply", 20.0, (Vector2){ btn_pos.x+600, btn_pos.y })) {
        set_render_dist(gst, gst->menu_slider_render_dist_v);
    }
    btn_pos.y += 50.0;


    /*

    if(gui_button(gst, "Toggle Fullscreen", 20.0, btn_pos)) {
        
        if(!IsWindowFullscreen()) {
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        }
        ToggleFullscreen();
        
        if(!IsWindowFullscreen()) {
            SetWindowSize(DEF_res_x, DEF_res_y);
        }
    }
    */


}


void gui_render_powerup_shop(struct state_t* gst) {
    struct powerup_shop_t* shop = &gst->player.powerup_shop;
   
    BeginShaderMode(gst->shaders[POWERUP_SHOP_BG_SHADER]);
    DrawRectangle(0, 0, gst->res_x, gst->res_y, (Color){ 0, 0, 0, 255 });
    EndShaderMode();

    const float offer_btn_space = 50.0;

    Vector2 shop_size = (Vector2) {
        740, (NUM_POWERUP_OFFERS * offer_btn_space)+150
    };

    Vector2 shop_pos = (Vector2) {
        200, 200
    };

    Rectangle shop_rect = (Rectangle){
        .x = shop_pos.x-50.0,
        .y = shop_pos.y-20.0,
        .width = shop_size.x,
        .height = shop_size.y
    };

    DrawRectangleV(
            (Vector2){
                shop_rect.x,
                shop_rect.y
            },
            shop_size,
            (Color){ 60, 45, 30, 200 });

    DrawRectangleLinesEx(shop_rect, 2.0, (Color){ 150, 50, 20, 200});

    DrawTextEx(gst->font, "* Powerup Shop Offers", shop_pos, 20.0, FONT_SPACING, TEXT_COLOR);
    
    DrawTextEx(gst->font, 
            TextFormat("Your XP: %i", gst->player.xp),
            (Vector2){shop_pos.x+20, shop_pos.y+35}, 15.0, FONT_SPACING, TEXT_COLOR);


   
    float offer_btn_y = shop_pos.y+80;

    int blink = (int)((sin(gst->time*10.0)*0.5+0.5)*100);
    Color selected_color = (Color){ 40, 100+blink, 50, 255 };
    const Color available_color = (Color){ 80, 80, 80, 255 };
    const Color unavailable_color = (Color){ 120, 50, 30, 255 };

    const Color focus_color = (Color) { 68, 50, 20, 255 };
    const Color unfocus_color = (Color) { 54, 41, 19, 255 };

    for(int i = 0; i < NUM_POWERUP_OFFERS; i++) {
        struct powerup_t* powerup = &shop->offers[i];

        const float offer_font_size = 20.0;
        const float space_xpcost_text = 180.0;
        Vector2 name_size = MeasureTextEx(gst->font, powerup->name, offer_font_size, FONT_SPACING);
        
        int can_afford = 
               (gst->xp_value_add == 0)
            && (powerup->xp_cost <= gst->player.xp)
            && (gst->player.powerup_levels[powerup->type] < powerup->max_level);


        Color ln_color = unavailable_color;
        if(can_afford) {
            ln_color = available_color;

            if(i == shop->selected_index) {
                ln_color = selected_color;
            }
        }


        DrawRectangleLinesEx(
                (Rectangle){
                shop_pos.x - EXTRA_OFF_W-4,
                offer_btn_y - EXTRA_OFF_H-4,
                name_size.x + EXTRA_OFF_W*2 + space_xpcost_text+8,
                name_size.y + EXTRA_OFF_H*1.5+8,
                },
                2.0,
                ln_color);

        DrawTextEx(gst->font,
                TextFormat("%i", powerup->xp_cost),
                (Vector2){
                    shop_pos.x - 5,
                    offer_btn_y
                }, 23.0, FONT_SPACING, (can_afford) ? TEXT_COLOR : (Color){ 100, 80, 80, 255 });

        if(gui_button_ext(gst, 
                    powerup->name, offer_font_size, 
                    (Vector2){ shop_pos.x+space_xpcost_text, offer_btn_y },
                    (Color){ 75, 45, 30, 200 },  // bg
                    can_afford ? (Color){ 200, 120, 60, 255 } : (Color){ 100, 88, 80, 200 }, // fg
                    focus_color, unfocus_color
                    ) 
                && can_afford) {
            printf("Selected '%s'\n", powerup->name);
            shop->selected_index = i;
        }

        offer_btn_y += offer_btn_space;
    }



    if(gui_button(gst, "Buy", 20.0, (Vector2){ shop_pos.x, offer_btn_y })) {
        if((shop->selected_index < 0) || (shop->selected_index >= NUM_POWERUP_OFFERS)) {
            return;
        }
        
        struct powerup_t* selected = &shop->offers[shop->selected_index];
        if(selected->xp_cost > gst->player.xp) {
            return;
        }

        shop->powerups[selected->type].xp_cost *= shop->powerups[selected->type].xp_cost_mult;

        apply_powerup(gst, &gst->player, selected->type);
        player_add_xp(gst, -selected->xp_cost);
        update_powerup_shop_offers(gst);
    }

    if(gui_button_ext(gst, 
                "New offers (-25 XP)", 15.0, 
                (Vector2){ shop_pos.x+150, offer_btn_y+5 },
                (Color){ 75, 45, 30, 200 },  // bg
                (Color){ 200, 120, 60, 255 }, // fg
                focus_color, unfocus_color) && (gst->player.xp >= 25)) {
        player_add_xp(gst, -25);
        update_powerup_shop_offers(gst);
    }

    if(gui_button_ext(gst, 
                "Close", 15.0, 
                (Vector2){ shop_pos.x+540, offer_btn_y+5 },
                (Color){ 75, 45, 30, 200 },  // bg
                (Color){ 200, 120, 60, 255 }, // fg
                focus_color, unfocus_color)) {
        shop->open = 0;
        DisableCursor();
    }


    Vector2 ptext_pos = (Vector2){ gst->res_x-500, 20 };
    for(int i = 0; i < NUM_POWERUPS; i++) {
        struct powerup_t* powerup = &gst->player.powerup_shop.powerups[i];

        DrawTextEx(gst->font,
                TextFormat("%i/%i (%s)", gst->player.powerup_levels[i], powerup->max_level, powerup->name),
                ptext_pos, 15.0, FONT_SPACING, TEXT_COLOR);

        ptext_pos.y += 20.0;
    }


}

void gui_render_devmenu(struct state_t* gst) {
    
    //DrawRectangle(0, 0, gst->res_x, gst->res_y, (Color){ 10, 30, 30, 200 });
    const char* menu_text = "[ Development menu ]";
    const float menu_text_fontsize = 20.0;
    DrawTextEx(gst->font, menu_text, 
            (Vector2){ gst->res_x - MeasureText(menu_text, menu_text_fontsize)*2, 10 },
            menu_text_fontsize, FONT_SPACING, TEXT_COLOR);


    const float btn_y_inc = 34.0;
    Vector2 btn_pos = (Vector2){ 80, 150 };


    if(gui_button(gst, "Telport to zero", 15.0, btn_pos)) {
        gst->player.cam.position = (Vector3){ 0, 0, 0 };
    }
    btn_pos.y += btn_y_inc;


    const float spawn_rad = 300.0;

    if(gui_button(gst, "Enemy LVL0", 15.0, btn_pos)) {
        spawn_enemy(gst, ENEMY_LVL0, ENT_FRIENDLY, (Vector3){
                    gst->player.position.x + RSEEDRANDOMF(-spawn_rad, spawn_rad),
                    0,
                    gst->player.position.z + RSEEDRANDOMF(-spawn_rad, spawn_rad)
                });
    }
    btn_pos.y += btn_y_inc;

    if(gui_button(gst, "Enemy LVL1", 15.0, btn_pos)) {
        spawn_enemy(gst, ENEMY_LVL1, ENT_FRIENDLY, (Vector3){
                    gst->player.position.x + RSEEDRANDOMF(-spawn_rad, spawn_rad),
                    0,
                    gst->player.position.z + RSEEDRANDOMF(-spawn_rad, spawn_rad)
                });
    }
    btn_pos.y += btn_y_inc;

    gui_checkbox(gst, "Render player gun (for debug)", 20.0, btn_pos, &gst->player.render);

    /*
    if(gui_button(gst, "RenderDist++", 15.0, btn_pos)) {
        set_render_dist(gst, gst->render_dist+200);
    }
    btn_pos.y += btn_y_inc;
    
    if(gui_button(gst, "RenderDist--", 15.0, btn_pos)) {
        set_render_dist(gst, gst->render_dist-200);
    }
    btn_pos.y += btn_y_inc;
    */
}

