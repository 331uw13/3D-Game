#include <math.h>
#include <stdio.h>

#include "state/state.h"
#include "gui_render.h"
#include "gui.h"
#include "util.h"

#include <raylib.h>


static void render_num_kills(struct state_t* gst) {

}


void gui_render_respawn_screen(struct state_t* gst) {
    render_num_kills(gst);

    struct guicfg_t guicfg;
    gui_load_default_cfg(&guicfg);
    
    guicfg.next_x = 100;
    guicfg.next_y = 200;

    gui_begin(gst);
    guicfg.font_size = 20;
    

    if(gui_button(gst, &guicfg, "Quit")) {
        gst->running = 0;
    }

    if(gui_button(gst, &guicfg, "Respawn")) {
        player_respawn(gst, &gst->player);
    }

    
    gui_end(gst);
}


void gui_render_menu_screen(struct state_t* gst) {
    render_num_kills(gst);


    struct guicfg_t guicfg;
    gui_load_default_cfg(&guicfg);
    guicfg.next_x = 100;
    guicfg.next_y = 200;
    guicfg.font_size = 21;


    gui_begin(gst);


    guicfg.font_size = 30;
    gui_text(gst, &guicfg, "Main Menu");

    guicfg.font_size = 15;
    gui_text(gst, &guicfg, "(Paused)");
    
    guicfg.font_size = 20;

    if(gui_button(gst, &guicfg, "Quit")) {
        gst->running = 0;
    }


    guicfg.next_y = gst->screen_size.y / 2;


    gui_sliderF(gst, &guicfg, /*ID*/0, "Render Distance", 500,
            &gst->menu_slider_render_dist_v,
            MIN_RENDERDIST, MAX_RENDERDIST);
    gui_next_x__previous(&guicfg);
    gui_next_y__previous(&guicfg);
    if(gui_button(gst, &guicfg, "Apply")) {
        set_render_dist(gst, gst->menu_slider_render_dist_v);
    }

    gui_next_x__saved(&guicfg, 0);
    
    gui_checkbox(gst, &guicfg, "Ambient Occlusion (ssao)", &gst->ssao_enabled);

    guicfg.next_y += 20;
    gui_end(gst);

}


static int g_wmodel_offset_settings_open = 0;
static int g_wmodel_stats_settings_open = 0;

void gui_render_devmenu(struct state_t* gst) {

    struct guicfg_t guicfg;
    gui_load_default_cfg(&guicfg);

    guicfg.font_size = 15;

    gui_begin(gst);

    guicfg.next_x = gst->screen_size.x - 450;
    
    // This is for creating a config for new weapon models in run time.
    // NOTE: (remember to uncomment section from 'state/state.c' update_frame() to apply these into current model.)
    gui_container(gst, &guicfg, "Weapon Model Offsets", GUI_CONTAINER_AUTO_SIZE, &g_wmodel_offset_settings_open);
    {
        
        gui_sliderF(gst, &guicfg, /*ID*/0, "aim_offset X", 400, &gst->testmd_aim_offset.x, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/1, "aim_offset Y", 400, &gst->testmd_aim_offset.y, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/2, "aim_offset Z", 400, &gst->testmd_aim_offset.z, -10.0, 10.0);


        gui_sliderF(gst, &guicfg, /*ID*/3, "rest_offset X", 400, &gst->testmd_rest_offset.x, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/4, "rest_offset Y", 400, &gst->testmd_rest_offset.y, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/5, "rest_offset Z", 400, &gst->testmd_rest_offset.z, -10.0, 10.0);


        gui_sliderF(gst, &guicfg, /*ID*/6, "rest_rotation X", 400, &gst->testmd_rest_rotation.x, -M_PI, M_PI);
        gui_sliderF(gst, &guicfg, /*ID*/7, "rest_rotation Y", 400, &gst->testmd_rest_rotation.y, -M_PI, M_PI);
        gui_sliderF(gst, &guicfg, /*ID*/8, "rest_rotation Z", 400, &gst->testmd_rest_rotation.z, -M_PI, M_PI);

        gui_sliderF(gst, &guicfg, /*ID*/9, "inspect_offset X", 400, &gst->testmd_inspect_offset.x, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/10, "inspect_offset Y", 400, &gst->testmd_inspect_offset.y, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/11, "inspect_offset Z", 400, &gst->testmd_inspect_offset.z, -10.0, 10.0);


        gui_sliderF(gst, &guicfg, /*ID*/12, "inspect_rotation X", 400, &gst->testmd_inspect_rotation.x, -M_PI, M_PI);
        gui_sliderF(gst, &guicfg, /*ID*/13, "inspect_rotation Y", 400, &gst->testmd_inspect_rotation.y, -M_PI, M_PI);
        gui_sliderF(gst, &guicfg, /*ID*/14, "inspect_rotation Z", 400, &gst->testmd_inspect_rotation.z, -M_PI, M_PI);


        gui_sliderF(gst, &guicfg, /*ID*/15, "light_offset X", 400, &gst->testmd_energy_light_pos.x, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/16, "light_offset Y", 400, &gst->testmd_energy_light_pos.y, -10.0, 10.0);
        gui_sliderF(gst, &guicfg, /*ID*/17, "light_offset Z", 400, &gst->testmd_energy_light_pos.z, -10.0, 10.0);
        
        gui_sliderF(gst, &guicfg, /*ID*/18, "prjfx_offset", 400, &gst->testmd_prjfx_offset, -30.0, 0.0);
    }
    gui_end_container(gst, &guicfg);
    

    gui_container(gst, &guicfg, "Weapon Model Stats", GUI_CONTAINER_AUTO_SIZE, &g_wmodel_stats_settings_open);
    {
        gui_sliderI(gst, &guicfg, /*ID*/19, "firerate", 400, &gst->testmd_firerate, 0, 30);
        gui_sliderI(gst, &guicfg, /*ID*/20, "accuracy", 400, &gst->testmd_accuracy, 0, 100);
        gui_sliderI(gst, &guicfg, /*ID*/21, "prj_speed", 400, &gst->testmd_prj_speed, 100, 1000);

    }
    gui_end_container(gst, &guicfg);

    if(g_wmodel_offset_settings_open || g_wmodel_stats_settings_open) {
        if(gui_button(gst, &guicfg, "config to stdout")) {
            printf(
                    "accuracy = %i;\n"
                    "firerate = %i;\n"
                    "projectile_speed = %i;\n"
                    "\n"
                    "aim_offset_x = %0.3f;\n"
                    "aim_offset_y = %0.3f;\n"
                    "aim_offset_z = %0.3f;\n"
                    "\n"
                    "rest_offset_x = %0.3f;\n"
                    "rest_offset_y = %0.3f;\n"
                    "rest_offset_z = %0.3f;\n"
                    "\n"
                    "rest_rotation_x = %0.3f;\n"
                    "rest_rotation_y = %0.3f;\n"
                    "rest_rotation_z = %0.3f;\n"
                    "\n"
                    "inspect_offset_x = %0.3f;\n"
                    "inspect_offset_y = %0.3f;\n"
                    "inspect_offset_z = %0.3f;\n"
                    "\n"
                    "inspect_rotation_x = %0.3f;\n"
                    "inspect_rotation_y = %0.3f;\n"
                    "inspect_rotation_z = %0.3f;\n"
                    "\n"
                    "energy_light_offset_x = %0.3f;\n"
                    "energy_light_offset_y = %0.3f;\n"
                    "energy_light_offset_z = %0.3f;\n"
                    "\n"
                    "prjfx_offset = %0.3f;\n"
                    ,

                    gst->testmd_accuracy,
                    gst->testmd_firerate,
                    gst->testmd_prj_speed,

                    gst->testmd_aim_offset.x,
                    gst->testmd_aim_offset.y,
                    gst->testmd_aim_offset.z,
                    
                    gst->testmd_rest_offset.x,
                    gst->testmd_rest_offset.y,
                    gst->testmd_rest_offset.z,
                    
                    gst->testmd_rest_rotation.x,
                    gst->testmd_rest_rotation.y,
                    gst->testmd_rest_rotation.z,

                    gst->testmd_inspect_offset.x,
                    gst->testmd_inspect_offset.y,
                    gst->testmd_inspect_offset.z,
                    
                    gst->testmd_inspect_rotation.x,
                    gst->testmd_inspect_rotation.y,
                    gst->testmd_inspect_rotation.z,

                    gst->testmd_energy_light_pos.x,
                    gst->testmd_energy_light_pos.y,
                    gst->testmd_energy_light_pos.z,

                    gst->testmd_prjfx_offset
                    );
        }
    }

    guicfg.next_x = 10;
    guicfg.next_y = 100;


    for(int i = 0; i < MAX_ENEMY_TYPES; i++) {
        if(gui_button(gst, &guicfg, TextFormat("Spawn ENEMY_LVL%i (Friendly)", i))) {
            const float radius = 200;
            Vector3 pos = (Vector3) {
                gst->player.position.x + RSEEDRANDOMF(-radius, radius),
                gst->player.position.y + RSEEDRANDOMF(-radius, radius),
                gst->player.position.z + RSEEDRANDOMF(-radius, radius)
            };
            chunk_add_enemy(gst, FIND_ENEMY_CHUNK, pos, ENEMY_LVL0 + i, ENT_FRIENDLY, ENEMY_ADDED_BY_SYSTEM);
        }
    }
    guicfg.next_y += 10;
    
    for(int i = 0; i < MAX_ENEMY_TYPES; i++) {
        if(gui_button(gst, &guicfg, TextFormat("Spawn ENEMY_LVL%i (Hostile)", i))) {
            const float radius = 800;
            Vector3 pos = (Vector3) {
                gst->player.position.x + RSEEDRANDOMF(-radius, radius),
                gst->player.position.y + RSEEDRANDOMF(-radius, radius),
                gst->player.position.z + RSEEDRANDOMF(-radius, radius)
            };
            chunk_add_enemy(gst, FIND_ENEMY_CHUNK, pos, ENEMY_LVL0 + i, ENT_HOSTILE, ENEMY_ADDED_BY_SYSTEM);
        }
    }


    if(gui_button(gst, &guicfg, "Limit FPS 25")) {
        SetTargetFPS(25);
    }
    if(gui_button(gst, &guicfg, "Limit FPS 500")) {
        SetTargetFPS(500);
    }

    guicfg.next_x = 10;
    if(gui_button(gst, &guicfg, "Teleport to 0,0,0")) {
        gst->player.cam.position = (Vector3){ 0, 0, 0 };
    }
    if(gui_button(gst, &guicfg, "Teleport to spawnpoint")) {
        gst->player.cam.position = gst->player.spawn_point;
    }

   
    gui_end(gst);
}


void render_crosshair_item_info(struct state_t* gst) {
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

    if(!inv->hovered_item) {
        return;
    }

    if(inv->hovered_item->empty || inv->hovered_item->inv_index < 0) {
        return;
    }

    if(!inv->hovered_item->info) {
        return;
    }


    const float desc_font_size = 13;
    const float name_font_size = 15;


    Vector2 mouse = GetMousePosition();
    Vector2 desc_size = MeasureTextEx(gst->font, inv->hovered_item->info->desc, desc_font_size, FONT_SPACING);
    Vector2 name_size = MeasureTextEx(gst->font, inv->hovered_item->info->name, name_font_size, FONT_SPACING);

    float info_box_width = (desc_size.x > name_size.x) ? desc_size.x : name_size.x;
    float info_box_height = desc_size.y + name_size.y;
    
    float info_box_x = mouse.x + 5;
    float info_box_y = mouse.y + 20;
    
    float padding = 10;



#define ADDTNBUF_MAX 1024
    // Additional info.
    char addtn_info[ADDTNBUF_MAX+1] = { 0 };
    size_t addtn_info_size = 0;

    if(inv->hovered_item->is_special) {
        get_item_additional_info(inv->hovered_item, addtn_info, ADDTNBUF_MAX, &addtn_info_size);
    
        addtn_info[addtn_info_size+1] = '\0';

        Vector2 addtn_info_size = MeasureTextEx(gst->font, addtn_info, desc_font_size, FONT_SPACING);
        if(info_box_width < addtn_info_size.x) {
            info_box_width = addtn_info_size.x;
        }

        info_box_height += addtn_info_size.y;
    }

    /*
    if(inv->hovered_item->is_weapon_item) {
        struct weapon_model_t* weapon_model = &inv->hovered_item->weapon_model;

        append_str(addtn_info, ADDTNBUF_MAX, &addtn_info_size, 
                TextFormat("Ammo: %i/%i\n", (int)weapon_model->stats.lqmag.ammo_level, (int)weapon_model->stats.lqmag.capacity));

        append_str(addtn_info, ADDTNBUF_MAX, &addtn_info_size,
                "Condition: (TODO!)\n");
        

        addtn_info[addtn_info_size+1] = '\0';
        
    }
    */


    float right_edge = info_box_x + info_box_width + padding + 20;

    if(right_edge > gst->screen_size.x) {
        info_box_x -= right_edge - gst->screen_size.x;
    }

    const Color line_color = (Color){ 0x74, 0x7A, 0x7A, 240 };
    const Color line2_color = (Color){ 0x46, 0x4D, 0x4D, 230 };
    float line_y = 0;


    DrawRectangle(
            info_box_x,
            info_box_y,
            info_box_width + padding * 2,
            info_box_height + padding * 2,
            (Color){ 23, 25, 27, 200 });


    // Item name.
    Vector2 name_pos = (Vector2){
        info_box_x + padding,
        info_box_y + padding
    };
    DrawTextEx(
            gst->font,
            inv->hovered_item->info->name,
            name_pos,
            name_font_size,
            FONT_SPACING,
            (Color){ 0x9E, 0xB2, 0xB6, 230 });


    line_y = name_pos.y + name_size.y + 1;
    DrawLine(info_box_x + 5, line_y, info_box_x + name_size.x + 20, line_y, line_color);


    // Item description.
    Vector2 desc_pos = (Vector2){
        info_box_x + padding,
        info_box_y + padding  + name_size.y + 5
    };
    DrawTextEx(
            gst->font,
            inv->hovered_item->info->desc,
            desc_pos,
            desc_font_size,
            FONT_SPACING,
            (Color){ 0x7E, 0x86, 0x86, 230 });

    // Additional info.
    if(addtn_info_size > 0) {

        line_y = desc_pos.y + desc_size.y + 3;
        DrawLine(info_box_x + 5, line_y, info_box_x + info_box_width + padding*2 - 10, line_y, line2_color);

        DrawTextEx(
                gst->font,
                addtn_info,
                (Vector2){
                    info_box_x + padding,
                    info_box_y + padding  + name_size.y + desc_size.y + 15
                },
                desc_font_size,
                FONT_SPACING,
                (Color){ 0x6E, 0x7D, 0x7D, 230 });

    }
}






