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


}

static int test_check = 0;
static int test_check2 = 1;
static float test_value = 32.0;
static float test_value2 = 0.0;
static float test_value3 = 0.0;
static int container_open = 1;
static int container2_open = 0;
static int container3_open = 1;

#include <rlgl.h>

void gui_render_menu_screen(struct state_t* gst) {
    render_num_kills(gst);


    struct guicfg_t guicfg;
    gui_load_default_cfg(&guicfg);
    guicfg.next_x = 100;
    guicfg.next_y = 100;
    guicfg.font_size = 21;


    gui_begin(gst);
    gui_text(gst, &guicfg, "Hello GUI?");


    gui_container(gst, &guicfg, "First Container", GUI_CONTAINER_AUTO_SIZE, &container_open);
    {

        if(gui_button(gst, &guicfg, "Test Button!")) {
            printf("Test button was clicked! %f\n", gst->time);
        }

        if(gui_checkbox(gst, &guicfg, "Checkbox", &test_check)) {
            printf("Checkbox: %i\n", test_check);
        }

        gui_checkbox(gst, &guicfg, "Enable", &test_check2);
        gui_checkbox(gst, &guicfg, "?", &test_check2);
        gui_button(gst, &guicfg, "Button");

    }
    gui_end_container(gst, &guicfg);


    gui_container(gst, &guicfg, "Container2", GUI_CONTAINER_AUTO_SIZE, &container2_open);
    {
        if(gui_button(gst, &guicfg, "Hello!")) {
            printf("Hello :)\n");
        }
    }
    gui_end_container(gst, &guicfg);


    gui_container(gst, &guicfg, "Container3", GUI_CONTAINER_AUTO_SIZE, &container3_open);
    {
        gui_text(gst, &guicfg, "Magic numbers");
    }
    gui_end_container(gst, &guicfg);



    gui_sliderF(gst, &guicfg, 0, "Angle", SLIDER_DEF_WIDTH, &test_value, 0.0, 100.0);
    gui_sliderF(gst, &guicfg, 1, "Distance", SLIDER_DEF_WIDTH, &test_value2, -1000.0, 10000.0);

    gui_next_x__previous(&guicfg);
    gui_next_y__previous(&guicfg);
    gui_sliderF(gst, &guicfg, 2, "Hmm", SLIDER_DEF_WIDTH, &test_value3, -M_PI, M_PI);

    gui_next_x__saved(&guicfg, 0);

    guicfg.next_y += 20;
    gui_text(gst, &guicfg, "Another text");


    gui_end(gst);

}



void gui_render_devmenu(struct state_t* gst) {

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


    /*
    if(gui_button(gst, "Drop", fontsize, pos)) {
        inv->selected_item->inv_index = -1;
        
        Vector3 drop_pos = gst->player.position;
        drop_pos.x += RSEEDRANDOMF(-3.0, 3.0);
        drop_pos.z += RSEEDRANDOMF(-3.0, 3.0);

        drop_item(gst, FIND_ITEM_CHUNK, drop_pos, inv->selected_item);
    }
    */


}


