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

    struct weapon_t gun;
    int  is_aiming;

    Model gun_model;
    Vector3 gun_model_poffset;

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
