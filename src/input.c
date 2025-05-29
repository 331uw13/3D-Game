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

void handle_userinput(struct state_t* gst) {
    
    if(gst->player.alive) {
        player_update_camera(gst, &gst->player);
        player_update_movement(gst, &gst->player);
    }


    if(!gst->player.any_gui_open && IsKeyPressed(KEY_E)) {
        gst->player.wants_to_pickup_item = 1;
    }

    // FOR TESTING
    {
        if(IsKeyPressed(KEY_U)) {
            spawn_item_type(gst, FIND_ITEM_CHUNK, gst->player.position, ITEM_APPLE, 1);
        }

    }


    if(gst->player.item_in_hands) {
        if(gst->player.item_in_hands->is_weapon_item) {
            if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)
            && !gst->player.inspecting_weapon
            && !gst->player.changing_item
            ){
                gst->player.is_aiming = !gst->player.is_aiming;
            }
            
            gst->player.inspecting_weapon = 0;
            if(IsKeyDown(KEY_F)) {
                gst->player.inspecting_weapon = 1;
            }
        }
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


    if(IsKeyPressed(KEY_TAB) && gst->player.alive) {
        toggle_gui(&gst->player.inventory.open);

        if(gst->player.inventory.open) {
            inventory_open_event(gst, &gst->player.inventory);
        }
        else {
            inventory_close_event(gst, &gst->player.inventory);
        }
        gst->devmenu_open = 0;
    }

    if(IsKeyPressed(KEY_ESCAPE) && gst->player.alive) {
        toggle_gui(&gst->menu_open);
        gst->devmenu_open = 0;
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

    if(IsKeyPressed(KEY_R) && !gst->player.any_gui_open) {
        if(!DEV_MODE) {
            fprintf(stderr, "\033[31mdev mode is disabled, cant open dev menu\033[0m\n");
            return;
        }
        toggle_gui(&gst->devmenu_open);
    }
}


