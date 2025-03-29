#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "state.h"
#include "util.h"

// TODO: clean this up.


static void toggle_gui(int* gui_open) {
    if((*gui_open = !*gui_open)) {
        EnableCursor();
    }
    else {
        DisableCursor();
    }
}


void handle_userinput(struct state_t* gst) {
    
    if(gst->player.alive) {
        player_update_camera(gst, &gst->player);
        player_update_movement(gst, &gst->player);
    }


    if(IsKeyPressed(KEY_F)) {
        player_respawn(gst, &gst->player);
    }


    if(IsKeyPressed(KEY_X)) {
        gst->player.weapon_firetype = !gst->player.weapon_firetype;
    }

    if(IsKeyPressed(KEY_C)) {
        spawn_item(gst, ITEM_METALPIECE, gst->player.cam.position);
    }
    if(IsKeyPressed(KEY_V)) {
        spawn_item(gst, ITEM_APPLE, gst->player.cam.position);
    }


    if(IsKeyPressed(KEY_F2)) {
        const char* filename = TextFormat("screenshot-%i.png", GetRandomValue(10000,99999));
        printf(" < Took screenshot, filename: \"%s\" >\n", filename);
        TakeScreenshot(filename);
    }

    if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        gst->player.aim_button_hold_timer += gst->dt;
        if(gst->player.aim_button_hold_timer >= 0.485/* <- Treshold */) {
            gst->player.disable_aim_mode = DISABLE_AIM_WHEN_RELEASED;
        }
    }
    else {
        gst->player.aim_button_hold_timer = 0.0;
        
    }

    // TODO: remove this.
    if(IsKeyPressed(KEY_I)) {
        update_powerup_shop_offers(gst);
    }
    
    if(gst->player.alive && gst->player.holding_gun) {
        if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) 
        && !gst->player.any_gui_open
        && (gst->player.disable_aim_mode == DISABLE_AIM_WHEN_MOUSERIGHT)) {
            gst->player.is_aiming =! gst->player.is_aiming;
            gst->player.aim_idle_timer = 0.0;
        }
        else
        if(!IsMouseButtonDown(MOUSE_RIGHT_BUTTON)
        && (gst->player.disable_aim_mode == DISABLE_AIM_WHEN_RELEASED)) {
            gst->player.is_aiming = 0;
        }
    }


    if(IsKeyPressed(KEY_ESCAPE) && gst->player.alive) {
        toggle_gui(&gst->menu_open);
        gst->player.powerup_shop.open = 0;
        gst->player.inventory.open = 0;
        gst->player.powerup_shop.open = 0;
        gst->devmenu_open = 0;
    }

    // TODO: remove this.
    if(IsKeyPressed(KEY_TWO)) {
        toggle_gui(&gst->player.powerup_shop.open);
        gst->player.powerup_shop.selected_index = -1;
        gst->player.inventory.open = 0;
    }
   


    // Dev mode input.

    if(IsKeyPressed(KEY_G)) {
        if(!DEV_MODE) {
            fprintf(stderr, "\033[31mdev mode is disabled, cant toggle noclip\033[0m\n");
            return;
        }
        gst->player.noclip = !gst->player.noclip;
    }

    if(IsKeyPressed(KEY_T)) {
        if(!DEV_MODE) {
            fprintf(stderr, "\033[31mdev mode is disabled, cant render debug info\033[0m\n");
            return;
        }
        gst->debug = !gst->debug;
        printf("\033[35m[\"DEBUG\"]: %i\033[0m\n", gst->debug);
    }

    if(IsKeyPressed(KEY_R)) {
        if(!DEV_MODE) {
            fprintf(stderr, "\033[31mdev mode is disabled, cant open dev menu\033[0m\n");
            return;
        }
        toggle_gui(&gst->devmenu_open);
    }


    if(IsKeyPressed(KEY_FOUR)) {
        SetTargetFPS(500);
    }
    if(IsKeyPressed(KEY_FIVE)) {
        SetTargetFPS(28);
    }

}


