#ifndef PLAYER_H
#define PLAYER_H


#include "weapon.h"

struct state_t;




struct player_t {

    Camera   cam;
    float cam_yaw;
    float cam_pitch;
    Vector3  position;
    Vector3  hitbox;
    Vector3  looking_at; // normalized vector where the player is looking towards

    //BoundingBox boundingbox;
    Vector3  velocity;
    float    walkspeed;
    float    run_mult;
    float    friction;
    float    jump_force;
    float    gravity;
    int      onground;

    int      max_jumps;
    int      num_jumps_inair;

    int      noclip;

    struct weapon_t weapon;
    int  is_aiming;

    Model gunmodel;
   
    //Vector3 gunmodel_offset;
    Matrix gunmodel_aim_offset_m;
    Matrix gunmodel_rest_offset_m;

    float gun_draw_timer; // 0.0 to 1.0 
    float gun_draw_speed; // how fast 'gun_draw_timer' reaches 1.0
    int   ready_to_shoot; // set to 1 when 'gun_draw_timer' finished.

};


void init_player_struct(struct state_t* gst, struct player_t* p);
void free_player(struct player_t* p);


void player_shoot(struct state_t* gst, struct player_t* p);
void player_render(struct state_t* gst, struct player_t* p);
void kill_projectile(struct state_t* gst, struct projectile_t* proj);
void update_projectiles(struct state_t* gst, struct player_t* p);

// updates player's variables. not movement
// for movement see (input.c)
void update_player(struct state_t* gst, struct player_t* p);



#endif
