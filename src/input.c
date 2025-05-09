#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "state/state.h"
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

#include "projectile_mod/prjmod_fmj_ability.h"

void handle_userinput(struct state_t* gst) {
    
    if(gst->player.alive) {
        player_update_camera(gst, &gst->player);
        player_update_movement(gst, &gst->player);
    }


    if(IsKeyPressed(KEY_E)) {
        gst->player.weapon.lqmag.ammo_level = gst->player.weapon.lqmag.capacity;
    }

    if(IsKeyPressed(KEY_F)) {
        gst->player.wants_to_pickup_item = 1;
    }

    if(IsKeyPressed(KEY_X)) {
        gst->player.weapon_firetype = !gst->player.weapon_firetype;
    }

    if(IsKeyPressed(KEY_U)) {
        spawn_item(gst, NULL, gst->player.position, ITEM_APPLE, 1);
    }

    if(gst->gamepad.id >= 0) {
        gst->gamepad.Lstick = (Vector2) {
            GetGamepadAxisMovement(gst->gamepad.id, GAMEPAD_AXIS_LEFT_X),
            GetGamepadAxisMovement(gst->gamepad.id, GAMEPAD_AXIS_LEFT_Y)
        };

        gst->gamepad.Rstick = (Vector2) {
            GetGamepadAxisMovement(gst->gamepad.id, GAMEPAD_AXIS_RIGHT_X),
            GetGamepadAxisMovement(gst->gamepad.id, GAMEPAD_AXIS_RIGHT_Y)
        };

        const float dead_rangeL = 0.125;
        const float dead_rangeR = 0.05;
        if(gst->gamepad.Lstick.x < dead_rangeL && gst->gamepad.Lstick.x > -dead_rangeL) {
            gst->gamepad.Lstick.x = 0.0;
        }
        if(gst->gamepad.Lstick.y < dead_rangeL && gst->gamepad.Lstick.y > -dead_rangeL) {
            gst->gamepad.Lstick.y = 0.0;
        }
        if(gst->gamepad.Rstick.x < dead_rangeR && gst->gamepad.Rstick.x > -dead_rangeR) {
            gst->gamepad.Rstick.x = 0.0;
        }
        if(gst->gamepad.Rstick.y < dead_rangeR && gst->gamepad.Rstick.y > -dead_rangeR) {
            gst->gamepad.Rstick.y = 0.0;
        }

        //printf("Lstick: %f, %f\n", gst->gamepad.Lstick.x, gst->gamepad.Lstick.y);
        //printf("Rstick: %f, %f\n", gst->gamepad.Rstick.x, gst->gamepad.Rstick.y);
        //printf("\n");
    }
    

    if(IsKeyPressed(KEY_F2)) {
        const char* filename = TextFormat("screenshot-%i.png", GetRandomValue(10000,99999));
        printf(" < Took screenshot, filename: \"%s\" >\n", filename);
        TakeScreenshot(filename);
    }


    if(gst->gamepad.id < 0) {
        // When controller is not detected use mouse input.

        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            gst->player.aim_button_hold_timer += gst->dt;
            if(gst->player.aim_button_hold_timer >= 0.485/* <- Treshold */) {
                gst->player.disable_aim_mode = DISABLE_AIM_WHEN_RELEASED;
            }
        }
        else {
            gst->player.aim_button_hold_timer = 0.0;
            
        }

        if(gst->player.alive && gst->player.holding_gun) {
            if((IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) 
            && !gst->player.any_gui_open
            && (gst->player.disable_aim_mode == DISABLE_AIM_WHEN_MOUSERIGHT)) {
                gst->player.is_aiming =! gst->player.is_aiming;
                gst->player.aim_idle_timer = 0.0;
            }
            else
            if((!IsMouseButtonDown(MOUSE_RIGHT_BUTTON)
            && (gst->player.disable_aim_mode == DISABLE_AIM_WHEN_RELEASED))) {
                gst->player.is_aiming = 0;
            }
        }
    }
    else {
        // Controller should have different system.

        int gamepad_aim_hold = (gst->gamepad.id >= 0 
                && IsGamepadButtonDown(gst->gamepad.id, GAMEPAD_BUTTON_LEFT_TRIGGER_1));

        if(gamepad_aim_hold) {
            if(!gst->player.is_aiming) {
                gst->player.is_aiming = 1;
                gst->player.aim_idle_timer = 0.0;
            }
        }
        else {
            gst->player.is_aiming = 0;
        }


    }

    if(IsKeyPressed(KEY_TAB) && gst->player.alive) {
        toggle_gui(&gst->player.inventory.open);
        gst->player.powerup_shop.open = 0;
        gst->devmenu_open = 0;
    }

    if(IsKeyPressed(KEY_ESCAPE) && gst->player.alive) {
        toggle_gui(&gst->menu_open);
        gst->player.powerup_shop.open = 0;
        gst->devmenu_open = 0;
    }

    // TODO: remove this.
    if(IsKeyPressed(KEY_TWO)) {
        toggle_gui(&gst->player.powerup_shop.open);
        gst->player.powerup_shop.selected_index = -1;
        gst->devmenu_open = 0;
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

    if(IsKeyPressed(KEY_R) && !gst->player.any_gui_open) {
        if(!DEV_MODE) {
            fprintf(stderr, "\033[31mdev mode is disabled, cant open dev menu\033[0m\n");
            return;
        }
        toggle_gui(&gst->devmenu_open);
    }

    /*
    if(IsKeyPressed(KEY_FOUR)) {
        SetTargetFPS(500);
    }
    if(IsKeyPressed(KEY_FIVE)) {
        SetTargetFPS(28);
    }
    */

}


