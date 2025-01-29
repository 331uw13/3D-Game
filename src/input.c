#include <raylib.h>
#include <rcamera.h>

#include "state.h"


#define CLAMP(v, min, max) ((v < min) ? min : (v > max) ? max : v)

#include <stdio.h>
void handle_userinput(struct state_t* gst) {

    Vector2 md = GetMouseDelta();
    float dt = GetFrameTime();

    CameraYaw(&gst->cam, (-md.x * CAMERA_SENSETIVITY) * dt, 0);
    CameraPitch(&gst->cam, (-md.y * CAMERA_SENSETIVITY) * dt, 1, 0, 0);

    
    float camspeed = gst->player.walkspeed;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        camspeed *= gst->player.run_mult;
    }
    
    camspeed *= dt;
    
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

    if(IsKeyPressed(KEY_SPACE) && gst->player.onground) {

        gst->player.velocity.y += gst->player.jump_force;
        gst->player.onground = 0;
    }

    // Y  movement.

    gst->player.velocity.y = CLAMP(gst->player.velocity.y,
            -5.0, 5.0);

    CameraMoveUp(&gst->cam, gst->player.velocity.y);


    if(gst->cam.position.y > 2) {
        gst->player.velocity.y -= gst->player.gravity * dt;
    }
    else {
        gst->player.onground = 1;
        gst->player.velocity.y = 0.0;
    }




    // X Z  movement.

    const float vmax = 1.0;

    gst->player.velocity.z = CLAMP(gst->player.velocity.z, -vmax, vmax);
    gst->player.velocity.x = CLAMP(gst->player.velocity.x, -vmax, vmax);

    CameraMoveForward(&gst->cam, gst->player.velocity.z, 1);
    CameraMoveRight(&gst->cam, gst->player.velocity.x, 1);
    


    const float f = (1.0 - gst->player.friction);
    gst->player.velocity.z *= f;
    gst->player.velocity.x *= f;


    gst->player.position = gst->cam.position;


    // ----- User interaction ---------



    if(IsKeyPressed(KEY_Y)) {
        gst->draw_debug = !gst->draw_debug;
    }

}







