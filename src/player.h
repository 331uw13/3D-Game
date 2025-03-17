#ifndef PLAYER_H
#define PLAYER_H


#include "weapon.h"
#include "light.h"
#include "inventory.h"
#include "item.h"

struct state_t;


#define PLAYER_HEALTH_COLOR_LOW (Color){ 255, 50, 20, 255 }
#define PLAYER_HEALTH_COLOR_HIGH (Color){ 50, 255, 20, 255 }

#define PLAYER_WEAPON_FULLAUTO 0
#define PLAYER_WEAPON_SEMIAUTO 1



struct player_t {

    Camera   cam;
    float cam_yaw;
    float cam_pitch;
    Vector3  position;
    Vector3  prev_position; // Previous frame position.
    Vector3  hitbox_size;
    float    hitbox_y_offset;
    float    height;
    Vector3  looking_at; // normalized vector where the player is looking towards
    int      is_moving;
    Vector3  velocity;
    float    walkspeed;
    float    walkspeed_aim_mult; // multiply walk speed while aiming
    float    run_mult;
    float    air_speed_mult;
    float    speed; // "Read only". Updated from 'input.c'
    float    ground_friction;
    float    air_friction;
    float    jump_force;
    float    gravity;
    int      onground;
    int      max_jumps;
    int      num_jumps_inair;
    int      noclip;
    int      is_aiming;
    int      alive;
    
    // External force. for example explosions..
    Vector3  ext_force_vel;
    Vector3  ext_force_acc;

    struct item_t* item_in_crosshair;

    struct inventory_t inventory;

    Model gunmodel;
    Material arms_material;
    struct light_t gun_light;

    //Vector3 gunmodel_offset;
    Matrix gunmodel_aim_offset_m;
    Matrix gunmodel_rest_offset_m;

    Vector3 rotation_from_hit; // Rotate player camera when player gets hit.

    // TODO: re write this recoil mess.
    float  recoil_timer;
    float  recoil_strength;
    float  recoil;
    int    recoil_done;
    int    recoil_in_progress;

    float gun_draw_timer; // 0.0 to 1.0 
    float gun_draw_speed; // how fast 'gun_draw_timer' reaches 1.0
    int   ready_to_shoot; // set to 1 when 'gun_draw_timer' finished.

    float health;
    float max_health;
    float health_normalized;

    struct weapon_t  weapon; // Weapon stats.
    struct psystem_t weapon_psys;
    float accuracy_modifier; // Used to decrease accuracy if shooting rapidly.
    float accuracy_control; // 'WEAPON_ACCURACY_MIN' to 'WEAPON_ACCURACY_MAX'. 
                            //  higher number means better accuracy control
    float time_from_last_shot;

    // FOR TESTING
    float gunfx_timer;
    Model gunfx_model;

    // Some weapon related variables may be stored else where.
    // For example the firerate because enemies use pointers to "global weapons"
    // and not their own weapon like the player has.
    float firerate_timer;
    float firerate;

    // Entities cant have this setting.
    int weapon_firetype; 
};


void init_player_struct(struct state_t* gst, struct player_t* p);
void delete_player(struct player_t* p);

void player_respawn(struct state_t* gst, struct player_t* p);
void player_shoot(struct state_t* gst, struct player_t* p);
void player_hit(struct state_t* gst, struct player_t* p, struct weapon_t* weapon);
void player_heal(struct state_t* gst, struct player_t* p, float heal);
void player_apply_force(struct state_t* gst, struct player_t* p, Vector3 force);
void player_update_death_animation(struct state_t* gst, struct player_t* p);

// updates player's variables. not movement for movement see (input.c)
void player_update(struct state_t* gst, struct player_t* p);
void player_render(struct state_t* gst, struct player_t* p);
void player_update_movement(struct state_t* gst, struct player_t* p);
void player_update_camera(struct state_t* gst, struct player_t* p);

BoundingBox get_player_boundingbox(struct player_t* p);


#endif
