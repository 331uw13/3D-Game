#include <raylib.h>
#include <rcamera.h>

#include "state.h"


#define CLAMP(v, min, max) ((v < min) ? min : (v > max) ? max : v)

#include <stdio.h>
void handle_userinput(struct state_t* gst) {


    // rotation and movement for the camera.
    Vector3 rot = (Vector3){ 0 };
    Vector3 mov = (Vector3){ 0 };


    Vector2 md = GetMouseDelta();

    float dt = GetFrameTime();

    CameraYaw(&gst->cam, (-md.x * CAMERA_SENSETIVITY) * dt, 0);
    CameraPitch(&gst->cam, (-md.y * CAMERA_SENSETIVITY) * dt, 1, 0, 0);

    
    float t = 0.9;
    float camspeed = gst->player_walkspeed;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        camspeed *= gst->player_run_mult;
    }
    
    camspeed *= dt;
    
    if(IsKeyDown(KEY_W)) {
        gst->player_velocity.z += camspeed;
    }
    if(IsKeyDown(KEY_S)) {
        gst->player_velocity.z -= camspeed;
    }
    if(IsKeyDown(KEY_A)) {
        gst->player_velocity.x -= camspeed;
    }
    if(IsKeyDown(KEY_D)) {
        gst->player_velocity.x += camspeed;
    }
    

    if(IsKeyPressed(KEY_SPACE) && gst->player_onground) {

        printf("jump.\n");

        gst->player_velocity.y += gst->player_jump_force;
        gst->player_onground = 0;
    }

    // Y  movement.

    gst->player_velocity.y = CLAMP(gst->player_velocity.y,
            -5.0, 5.0);

    CameraMoveUp(&gst->cam, gst->player_velocity.y);


    if(gst->cam.position.y > 0) {
        gst->player_velocity.y -= gst->player_gravity * dt;
    }
    else {
        gst->player_onground = 1;
        gst->player_velocity.y = 0.0;
    }






    // X Z  movement.

    const float vmax = 1.0;

    gst->player_velocity.z = CLAMP(gst->player_velocity.z, -vmax, vmax);
    gst->player_velocity.x = CLAMP(gst->player_velocity.x, -vmax, vmax);

    CameraMoveForward(&gst->cam, gst->player_velocity.z, 1);
    CameraMoveRight(&gst->cam, gst->player_velocity.x, 1);
    


    gst->player_velocity.z *= t;
    gst->player_velocity.x *= t;


    /*
    if(IsKeyDown(KEY_S)) {
            CameraMoveForward(&gst->cam, -camspeed, 1);
    }
    if(IsKeyDown(KEY_A)) {
        CameraMoveRight(&gst->cam, -camspeed, 1);
    }
    if(IsKeyDown(KEY_D)) {
        CameraMoveRight(&gst->cam,  camspeed, 1);
    }
    */



}

