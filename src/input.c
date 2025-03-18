#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "state.h"
#include "util.h"

// TODO: clean this up.


void handle_userinput(struct state_t* gst) {
    
    if(gst->player.alive) {
        player_update_camera(gst, &gst->player);
        player_update_movement(gst, &gst->player);
    }


    if(IsKeyPressed(KEY_F)) {
        player_respawn(gst, &gst->player);
    }



    if(IsKeyPressed(KEY_G)) {
        gst->player.noclip = !gst->player.noclip;
    }

    if(IsKeyPressed(KEY_X)) {
        gst->player.weapon_firetype = !gst->player.weapon_firetype;
    }


    if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        gst->player.aim_button_hold_timer += gst->dt;
        if(gst->player.aim_button_hold_timer >= 0.485) {
            gst->player.disable_aim_mode = DISABLE_AIM_WHEN_RELEASED;
        }
    }
    else {
        gst->player.aim_button_hold_timer = 0.0;
        
    }

    if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) 
    && !gst->player.inventory.open
    && (gst->player.disable_aim_mode == DISABLE_AIM_WHEN_MOUSERIGHT)) {
        gst->player.is_aiming =! gst->player.is_aiming;
    }
    else
    if(!IsMouseButtonDown(MOUSE_RIGHT_BUTTON)
    && (gst->player.disable_aim_mode == DISABLE_AIM_WHEN_RELEASED)) {
        gst->player.is_aiming = 0;
    }

    

    if(IsKeyPressed(KEY_C)) {
        spawn_item(gst, ITEM_APPLE, APPLE_INV_TEXID, ITEM_COMMON, gst->player.position);
    }

    if(IsKeyPressed(KEY_FIVE)) {
        spawn_enemy(gst, ENEMY_LVL0, ENT_HOSTILE, 
                (Vector3){
                    gst->player.position.x + RSEEDRANDOMF(-100, 100),
                    0,
                    gst->player.position.z + RSEEDRANDOMF(-100, 100)
                });

    }
   

    if(IsKeyPressed(KEY_T)) {
        gst->debug = !gst->debug;
        printf("\033[35m[\"DEBUG\"]: %i\033[0m\n", gst->debug);
    }

    if(IsKeyPressed(KEY_TAB)) {
        toggle_inventory(gst, &gst->player);
    }

    if(IsKeyPressed(KEY_ONE)) {
        SetTargetFPS(35);
        printf("Target FPS 35 (for debug) press <2> to undo.\n");
    }
    if(IsKeyPressed(KEY_TWO)) {
        SetTargetFPS(TARGET_FPS);
        printf("Target FPS %i\n", TARGET_FPS);
    }


}


