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

    if(IsKeyDown(KEY_LEFT_SHIFT) && !gst->player.is_aiming) {
        camspeed *= gst->player.run_mult;
    }
    
    camspeed *= dt;
   
        
    // ----- Handle player Y movement -------
    //


    // TODO: CLEAN THIS SHIT.
    // - fix terrain max y thing. clips into mesh.
    // - make this helper function for enemies to use too.
    // - clean enemy stuff too :)

    if(!gst->player.noclip)
    {

        if(IsKeyPressed(KEY_T)) {
            gst->player.position.y = 10;
        }

        Vector3 pos = gst->player.position;


        // pretty expensive to compute for bigger terrain
        // but will work around that someday.

        /*
        printf("%f\n", gst->terrain.highest_point);

        ray = (Ray) {
            (Vector3) {
                gst->player.position.x,
                gst->terrain.highest_point,
                gst->player.position.z
            },
            (Vector3) { 0.0, -0.99, 0.0 }
        };
        RayCollision mesh_info = { 0 };

        //mesh_info = GetRayCollisionTriangle(ray, v1,v2,v3);
        mesh_info = GetRayCollisionMesh(ray, gst->terrain.mesh, gst->terrain.transform);
        

        gst->player.position.y = ((gst->terrain.highest_point+4)-mesh_info.distance);

        */

        RayCollision t_hit = raycast_terrain(&gst->terrain, gst->player.position.x, gst->player.position.z);
        gst->player.position.y = t_hit.point.y + gst->player.hitbox_size.y;


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



    /*
    // ----- Handle player Y movement -------
    if(!gst->player.noclip) {
        if(IsKeyPressed(KEY_SPACE) && gst->player.onground) {

            gst->player.velocity.y = 0;
            gst->player.velocity.y += gst->player.jump_force;
            
            gst->player.num_jumps_inair++;
            if(gst->player.num_jumps_inair >= gst->player.max_jumps) {
                gst->player.onground = 0;
            }
        }

        gst->player.velocity.y = CLAMP(gst->player.velocity.y,
                -5.0, 5.0);

        CameraMoveUp(&gst->player.cam, gst->player.velocity.y);

        int ix = round((int)gst->player.position.x);
        int iz = round((int)gst->player.position.z);

        float heightpoint = 2.0*get_heightmap_value(&gst->terrain, ix, iz);

        printf("%f\n", heightpoint);

        if(gst->player.cam.position.y > (2.1 + heightpoint)) {
            gst->player.velocity.y -= gst->player.gravity * dt;
        }
        else {
            // Player is on ground
            gst->player.onground = 1;
            gst->player.velocity.y = 0.0;
            gst->player.num_jumps_inair = 0;
        }
        
    }
    else {
       
    }
    */



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

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        player_shoot(gst, &gst->player);
    }

    if(IsKeyPressed(KEY_G)) {
        gst->player.noclip = !gst->player.noclip;
    }


    int aimkeydown = IsKeyDown(KEY_LEFT_CONTROL);
    gst->player.is_aiming = aimkeydown;

    if(!aimkeydown) {
        gst->player.ready_to_shoot = 0;
    }


    if(IsKeyPressed(KEY_T)) {
        gst->debug = !gst->debug;
    }
}







