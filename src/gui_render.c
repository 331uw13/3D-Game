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
    
    gui_slider_float(gst, "Render Distance", 20.0, btn_pos, 530,
            &gst->menu_slider_render_dist_v, MIN_RENDERDIST, MAX_RENDERDIST);
    if(gui_button(gst, "Apply", 20.0, (Vector2){ btn_pos.x+600, btn_pos.y })) {
        set_render_dist(gst, gst->menu_slider_render_dist_v);
    }
    btn_pos.y += 50.0;
    
    gui_slider_float(gst, "Controller Sensetivity", 20.0, btn_pos, 530,
            &gst->gamepad.sensetivity, 1.0, 10.0);

}



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
 
    if(gui_button(gst, "Toggle SSAO View", fontsize, btn_pos)) {
        gst->show_only_ssao = !gst->show_only_ssao;
        shader_setu_int(gst, POSTPROCESS_SHADER, U_ONLY_SSAO, &gst->show_only_ssao);
        printf("vitun vitun vittu\n");
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


    btn_pos.x = gst->screen_size.x-400;
    btn_pos.y = 200;

    gui_slider_float(gst, "OffsetX", fontsize, btn_pos, 300,
            &gst->test_model_offset.x, -10.0, 10.0);
    btn_pos.y += btn_y_inc+5;
    gui_slider_float(gst, "OffsetY", fontsize, btn_pos, 300,
            &gst->test_model_offset.y, -10.0, 10.0);
    btn_pos.y += btn_y_inc+5;
    gui_slider_float(gst, "OffsetZ", fontsize, btn_pos, 300,
            &gst->test_model_offset.z, -10.0, 10.0);
    btn_pos.y += btn_y_inc+5;


    gui_slider_float(gst, "RotationX", fontsize, btn_pos, 300,
            &gst->test_model_rotation.x, -M_PI, M_PI);
    btn_pos.y += btn_y_inc+5;
    gui_slider_float(gst, "RotationY", fontsize, btn_pos, 300,
            &gst->test_model_rotation.y, -M_PI, M_PI);
    btn_pos.y += btn_y_inc+5;
    gui_slider_float(gst, "RotationZ", fontsize, btn_pos, 300,
            &gst->test_model_rotation.z, -M_PI, M_PI);
    btn_pos.y += btn_y_inc+5;


    if(gui_button(gst, "^- Print", fontsize, btn_pos)) {
        printf("Offset: (%0.4f, %0.4f, %0.4f)\n",
                gst->test_model_offset.x,
                gst->test_model_offset.y,
                gst->test_model_offset.z
                );

        printf("Rotation: (%0.4f, %0.4f, %0.4f)\n",
                gst->test_model_rotation.x,
                gst->test_model_rotation.y,
                gst->test_model_rotation.z
                );
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


void render_item_info(struct state_t* gst) {
    if(!gst->crosshair_item_info) {
        return;
    }


    Vector2 text_pos = (Vector2) {
        20, 170
    };

    const float origin_y = text_pos.y;

    const int name_text_font_size = 16;
    const int desc_text_font_size = 13;

    Vector2 name_text_m = MeasureTextEx(gst->font, 
            gst->crosshair_item_info->name,
            name_text_font_size,
            FONT_SPACING);

    Vector2 desc_text_m = MeasureTextEx(gst->font, 
            gst->crosshair_item_info->desc,
            desc_text_font_size,
            FONT_SPACING);

    gst->item_info_screen_time += gst->dt;
    const int box_final_height = 150;
    float box_height = CLAMP(gst->item_info_screen_time * 450, 0, box_final_height);
    float box_width = 485;

    float name_text_alpha = CLAMP(gst->item_info_screen_time * 400, 0, 255);
    float desc_text_alpha = CLAMP(gst->item_info_screen_time * 230, 0, 255);
    float act_text_alpha = CLAMP(gst->item_info_screen_time * 400, 0, 255);

    float act_box_width = CLAMP(gst->item_info_screen_time * 850, 0, box_width - 50);


    DrawRectangle(0, text_pos.y - 10, box_width, box_height, (Color){ 30, 40, 45, 160 });
    DrawTextEx(gst->font, gst->crosshair_item_info->name, text_pos,
            name_text_font_size, FONT_SPACING,
            (Color){ 200, 200, 200, name_text_alpha });

    DrawLine(0, text_pos.y+name_text_m.y, box_width, text_pos.y+name_text_m.y, 
            (Color){ 70, 80, 85, 255 });

    text_pos.y += name_text_m.y + 5;

    DrawTextEx(gst->font, gst->crosshair_item_info->desc, text_pos,
            desc_text_font_size, FONT_SPACING,
            (Color){ 140, 150, 186, desc_text_alpha });

    text_pos.y = origin_y + box_final_height;
    DrawRectangle(0, text_pos.y-5, act_box_width, 20, (Color){ 30, 70, 90, 70 });

    DrawTextEx(gst->font, "Press <E> to Pickup", text_pos,
            15, FONT_SPACING,
            (Color){ 70, 80, 90, act_text_alpha });
}

void gui_render_inventory_controls(struct state_t* gst, struct inventory_t* inv) {
    float fontsize = 15;
    Vector2 pos = (Vector2) { 25, 200 };

    DrawRectangle(0, pos.y-20, 250, 400, (Color){ 20, 20, 20, 200 });

    if(!inv->selected_item) {
        return;
    }

    if(inv->selected_item->inv_index < 0) {
        return;
    }

    DrawTextEx(gst->font, inv->selected_item->info->name, pos,
            15, FONT_SPACING,
            (Color){ 200, 200, 200, 255 });


    pos.x += 20;
    pos.y += 40;


    if(gui_button(gst, "Drop", fontsize, pos)) {
        inv->selected_item->inv_index = -1;
        
        Vector3 drop_pos = gst->player.position;
        drop_pos.x += RSEEDRANDOMF(-3.0, 3.0);
        drop_pos.z += RSEEDRANDOMF(-3.0, 3.0);

        drop_item(gst, FIND_ITEM_CHUNK, drop_pos, inv->selected_item);
    }


}


