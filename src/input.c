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

    int aimkeydown = IsKeyDown(KEY_LEFT_CONTROL) && !gst->player.inventory.open;
    gst->player.is_aiming = aimkeydown;

    if(!aimkeydown) {
        gst->player.ready_to_shoot = 0;
    }


    if(IsKeyPressed(KEY_C)) {
        spawn_item(gst, ITEM_APPLE, APPLE_INV_TEXID, ITEM_COMMON, gst->player.position);
    }

    if(IsKeyPressed(KEY_FIVE)) {
        spawn_enemy(gst, ENEMY_LVL0, 50, ENT_HOSTILE, 
                (Vector3){
                    gst->player.position.x + RSEEDRANDOMF(-10, 10),
                    0,
                    gst->player.position.z + RSEEDRANDOMF(-10, 10)
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


