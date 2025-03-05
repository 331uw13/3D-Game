#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "state.h"
#include "util.h"




void handle_userinput(struct state_t* gst) {

    Vector2 md = GetMouseDelta();
    float dt = GetFrameTime();

    CameraYaw(&gst->player.cam, (-md.x * CAMERA_SENSETIVITY), 0);
    CameraPitch(&gst->player.cam, (-md.y * CAMERA_SENSETIVITY), 1, 0, 0);

    gst->player.cam_yaw = (-md.x * CAMERA_SENSETIVITY) * dt;
    gst->player.looking_at = Vector3Normalize(Vector3Subtract(gst->player.cam.target, gst->player.cam.position));

    
    float camspeed = gst->player.walkspeed;

    if((IsKeyDown(KEY_LEFT_SHIFT) && !gst->player.is_aiming && gst->player.onground)) {
        camspeed *= gst->player.run_mult;
    }

    if(!gst->player.onground) {
        camspeed *= gst->player.air_speed_mult;
    }
    
    camspeed *= dt;
   
        
    // ----- Handle player Y movement -------
    //

    if(!gst->player.noclip)
    {

        if(IsKeyPressed(KEY_SPACE) && gst->player.onground) {
            gst->player.velocity.y = gst->player.jump_force;
            gst->player.onground = 0;
        }

        Vector3 pos = gst->player.position;


        RayCollision t_hit = raycast_terrain(&gst->terrain, gst->player.position.x, gst->player.position.z);
        const float heightvalue = t_hit.point.y + gst->player.height;
        //gst->player.position.y = t_hit.point.y + gst->player.hitbox_size.y;

        gst->player.position.y += gst->player.velocity.y;
        
        if((gst->player.position.y < heightvalue) || gst->player.onground) {
            gst->player.position.y = heightvalue;
            gst->player.velocity.y = 0;
            gst->player.onground = 1;
        }
        
        if(!gst->player.onground){
            gst->player.velocity.y -= gst->player.gravity * gst->dt;
        }

        float scale_up = ( gst->player.position.y - gst->player.cam.position.y);

        Vector3 up = Vector3Scale(GetCameraUp(&gst->player.cam), scale_up);
        gst->player.cam.target = Vector3Add(gst->player.cam.target, up);

        gst->player.cam.position.y = gst->player.position.y;


    }
    else {
        const float noclip_speed_mult = 3;
        if(IsKeyDown(KEY_SPACE)) {
            CameraMoveUp(&gst->player.cam, (dt * 15.0) * noclip_speed_mult);
        }
        else if(IsKeyDown(KEY_LEFT_CONTROL)) {
            CameraMoveUp(&gst->player.cam, -(dt * 18.0) * noclip_speed_mult);
        }
        camspeed *= noclip_speed_mult;
    }
 

    // XZ Movement.

    if(IsKeyDown(KEY_W)) {
        gst->player.velocity.z += camspeed;
    }
    if(IsKeyDown(KEY_S)) {
        gst->player.velocity.z -= camspeed;
    }
    if(IsKeyDown(KEY_A)) {
        gst->player.velocity.x -= camspeed;
    }
    if(IsKeyDown(KEY_D)) {
        gst->player.velocity.x += camspeed;
    }




    // X Z  movement.

    const float vmax = 1.0;

    gst->player.velocity.z = CLAMP(gst->player.velocity.z, -vmax, vmax);
    gst->player.velocity.x = CLAMP(gst->player.velocity.x, -vmax, vmax);

    CameraMoveForward(&gst->player.cam, gst->player.velocity.z, 1);
    CameraMoveRight(&gst->player.cam, gst->player.velocity.x, 1);
    

    const float f = (1.0 - gst->player.friction);
    gst->player.velocity.z *= f;
    gst->player.velocity.x *= f;


    gst->player.position = gst->player.cam.position;

    // ----- User interaction ---------

    int shoot = 
          (gst->player.weapon_firetype == PLAYER_WEAPON_FULLAUTO)
        ? (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        : (IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if(shoot) {
        player_shoot(gst, &gst->player);
    }

    if(IsKeyPressed(KEY_G)) {
        gst->player.noclip = !gst->player.noclip;
    }

    if(IsKeyPressed(KEY_X)) {
        gst->player.weapon_firetype = !gst->player.weapon_firetype;
    }

    int aimkeydown = IsKeyDown(KEY_LEFT_CONTROL);
    gst->player.is_aiming = aimkeydown;

    if(!aimkeydown) {
        gst->player.ready_to_shoot = 0;
    }

    if(IsKeyPressed(KEY_T)) {
        gst->debug = !gst->debug;
        printf("\033[35m[\"DEBUG\"]: %i\033[0m\n", gst->debug);
    }
}


