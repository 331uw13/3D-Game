#include <math.h>
#include <stdio.h>

#include "state/state.h"
#include "gui_render.h"
#include "gui.h"
#include "util.h"

#include <raylib.h>


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

    gui_checkbox(gst, "Ambient Occlusion", 20.0, btn_pos, &gst->ssao_enabled);
    btn_pos.y += 50.0;
    
    gui_checkbox(gst, "Render Grass", 20.0, btn_pos, &gst->grass_enabled);
    btn_pos.y += 50.0;

    gui_slider_float(gst, "Render Distance", 20.0, btn_pos, 530,
            &gst->menu_slider_render_dist_v, MIN_RENDERDIST, MAX_RENDERDIST);
    if(gui_button(gst, "Apply", 20.0, (Vector2){ btn_pos.x+600, btn_pos.y })) {
        set_render_dist(gst, gst->menu_slider_render_dist_v);
    }
    btn_pos.y += 50.0;
    
    gui_slider_float(gst, "Grass Render Distance", 20.0, btn_pos, 530,
            &gst->terrain.grass_render_dist, 1000, gst->render_dist);
    
    btn_pos.y += 50.0;

    gui_slider_float(gst, "Controller Sensetivity", 20.0, btn_pos, 530,
            &gst->gamepad.sensetivity, 1.0, 10.0);

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
 
    /*
    BeginShaderMode(gst->shaders[POWERUP_SHOP_BG_SHADER]);
    DrawRectangle(0, 0, gst->res_x, gst->res_y, (Color){ 0, 0, 0, 255 });
    EndShaderMode();
    */
    
    DrawRectangle(0, 0, gst->res_x, gst->res_y, (Color){ 52, 32, 10, 150 });
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


    for(int i = 0; i < NUM_POWERUP_OFFERS; i++) {
        struct powerup_t* powerup = &shop->offers[i];

        const float offer_font_size = 20.0;
        const float space_xpcost_text = 180.0;
        Vector2 name_size = MeasureTextEx(gst->font, powerup->name, offer_font_size, FONT_SPACING);

        int max_level = (gst->player.powerup_levels[powerup->type] >= powerup->max_level);
        int can_afford = 
               (gst->xp_update_done)
            && (powerup->xp_cost <= gst->player.xp)
            && !max_level;


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
                max_level ? "(Max)" : TextFormat("%i", powerup->xp_cost),
                (Vector2){
                    shop_pos.x - 5,
                    offer_btn_y
                }, 23.0, FONT_SPACING, (can_afford) ? TEXT_COLOR : (Color){ 100, 80, 80, 255 });

        if(gui_button(gst, 
                    powerup->name, offer_font_size, 
                    (Vector2){ shop_pos.x+space_xpcost_text, offer_btn_y }) 
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

    if(gui_button(gst, "New offers (-25 XP)", 15.0, 
                (Vector2){ shop_pos.x+150, offer_btn_y+5 })
            && (gst->player.xp >= 25)) {
        player_add_xp(gst, -25);
        update_powerup_shop_offers(gst);
    }

    if(gui_button(gst, "Close", 15.0, 
                (Vector2){ shop_pos.x+540, offer_btn_y+5 })) {
        shop->open = 0;
        DisableCursor();
    }


    Vector2 ptext_pos = (Vector2){ gst->res_x-500, 20 };
    for(int i = 0; i < MAX_POWERUP_TYPES; i++) {
        struct powerup_t* powerup = &gst->player.powerup_shop.powerups[i];

        DrawTextEx(gst->font,
                TextFormat("%i/%i (%s)", gst->player.powerup_levels[i], powerup->max_level, powerup->name),
                ptext_pos, 15.0, FONT_SPACING, TEXT_COLOR);

        ptext_pos.y += 20.0;
    }
}

static int g_show_biome_map = 0; // Temporary.


void gui_render_devmenu(struct state_t* gst) {
    const float fontsize = 15;
    //DrawRectangle(0, 0, gst->res_x, gst->res_y, (Color){ 10, 30, 30, 200 });
    const char* menu_text = "[ Development menu ]";
    const float menu_text_fontsize = 20.0;
    DrawTextEx(gst->font, menu_text, 
            (Vector2){ gst->res_x - MeasureText(menu_text, menu_text_fontsize)*2, 10 },
            menu_text_fontsize, FONT_SPACING, TEXT_COLOR);


    const float btn_y_inc = 34.0;
    const float space_y = 20.0;
    Vector2 btn_pos = (Vector2){ 100, 150 };


    if(gui_button(gst, "Telport to spawn point", fontsize, btn_pos)) {
        gst->player.cam.position = gst->player.spawn_point;
    }
    btn_pos.y += btn_y_inc;
    if(gui_button(gst, "Telport to zero", fontsize, btn_pos)) {
        gst->player.cam.position = (Vector3){ 0, 0, 0 };
    }
    btn_pos.y += btn_y_inc;
 
    if(gui_button(gst, "Telport to first chunk", fontsize, btn_pos)) {
        gst->player.cam.position = gst->terrain.chunks[0].center_pos;
    }
    btn_pos.y += btn_y_inc;
 

    if(gui_button(gst, "Kill player", fontsize, btn_pos)) {
        player_damage(gst, &gst->player, 99999999);
        gst->devmenu_open = 0;
    }
    btn_pos.y += btn_y_inc;
    btn_pos.y += space_y;

    if(gui_button(gst, "Spawn NPC", fontsize, btn_pos)) {
        gst->npc.position = (Vector3){
            gst->player.position.x + RSEEDRANDOMF(-30, 30),
            0,
            gst->player.position.z + RSEEDRANDOMF(-30, 30)
        };
        gst->npc.travel.dest_reached = 1;
        gst->npc.active = 1;
    }
    btn_pos.y += btn_y_inc;



    const float spawn_rad = 300.0;

    for(int i = 0; i < MAX_ENEMY_TYPES; i++) {
        if(gui_button(gst, TextFormat("Spawn Enemy LVL%i (friendly)", i), fontsize, btn_pos)) {
            spawn_enemy(gst, i, ENT_FRIENDLY, (Vector3){
                        gst->player.position.x + RSEEDRANDOMF(-spawn_rad, spawn_rad),
                        0,
                        gst->player.position.z + RSEEDRANDOMF(-spawn_rad, spawn_rad)
                    });
        }
        btn_pos.y += btn_y_inc;
    }

    btn_pos.y += space_y;

    for(int i = 0; i < MAX_ENEMY_TYPES; i++) {
        if(gui_button(gst, TextFormat("Spawn Enemy LVL%i (hostile)", i), fontsize, btn_pos)) {
            spawn_enemy(gst, i, ENT_HOSTILE, (Vector3){
                        gst->player.position.x + RSEEDRANDOMF(-spawn_rad+100, spawn_rad+100),
                        0,
                        gst->player.position.z + RSEEDRANDOMF(-spawn_rad+100, spawn_rad+100)
                    });
        }
        btn_pos.y += btn_y_inc;
    }
    
    btn_pos.y += space_y*2;

    gui_slider_float(gst, "Wind Direction X", fontsize, btn_pos, 300,
            &gst->weather.wind_dir.x, -1.0, 1.0);
    btn_pos.y += btn_y_inc+5;
    
    gui_slider_float(gst, "Wind Direction Z", fontsize, btn_pos, 300,
            &gst->weather.wind_dir.z, -1.0, 1.0);
    btn_pos.y += btn_y_inc+5;

    gui_slider_float(gst, "Wind Strength", fontsize, btn_pos, 300,
            &gst->weather.wind_strength, 0.0, 600.0);
    btn_pos.y += btn_y_inc+5;

    
    if(gui_button(gst, "Show biome map", fontsize, btn_pos)) {
        g_show_biome_map = 1;
    }


    btn_pos.y += btn_y_inc;

    /*
    // ------------ Color pickers -----------

    if(gui_colorpicker(gst, "Fog Top color", 
                (Vector2){ gst->res_x-300, 100 }, (Vector2){ 200, 200 },
                &gst->fog.color_top)) {      
        //printf("%i, %i, %i\n", gst->fog.color_bottom.r, gst->fog.color_bottom.g, gst->fog.color_bottom.b);
        set_fog_settings(gst, &gst->fog);
        printf("Fog top color: (%i, %i, %i)\n", gst->fog.color_top.r, gst->fog.color_top.g, gst->fog.color_top.b);
    }

    if(gui_colorpicker(gst, "Fog Bottom color", 
                (Vector2){ gst->res_x-300, 350 }, (Vector2){ 200, 200 },
                &gst->fog.color_bottom)) {      
        //printf("%i, %i, %i\n", gst->fog.color_bottom.r, gst->fog.color_bottom.g, gst->fog.color_bottom.b);
        set_fog_settings(gst, &gst->fog);
        printf("Fog bottom color: (%i, %i, %i)\n", gst->fog.color_bottom.r, gst->fog.color_bottom.g, gst->fog.color_bottom.b);
    }

    if(gui_colorpicker(gst, "Sun color", 
                (Vector2){ gst->res_x-550, 100 }, (Vector2){ 200, 200 },
                &gst->weather.sun_color)) {      
        //printf("%i, %i, %i\n", gst->fog.color_bottom.r, gst->fog.color_bottom.g, gst->fog.color_bottom.b);
        set_fog_settings(gst, &gst->fog);
        printf("Fog bottom color: (%i, %i, %i)\n", gst->fog.color_bottom.r, gst->fog.color_bottom.g, gst->fog.color_bottom.b);
    }
    */


    


}

