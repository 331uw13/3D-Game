#ifndef PLAYER_H
#define PLAYER_H


#include "weapon.h"
#include "light.h"
#include "inventory.h"
#include "item.h"
#include "powerup.h"

struct state_t;


#define ABS_MAX_ARMOR 20
#define ABS_MAX_HEALTH 1000
#define MAX_DEFAULT_ARMOR  3
#define DEFAULT_ARMOR_DAMAGE_DAMPEN 0.765

#define ARMOR_DAMAGE_DAMPEN_MIN 0.95
#define ARMOR_DAMAGE_DAMPEN_MAX 0.25

#define FIRERATE_MIN 0.0005
#define FIRERATE_MAX 0.1


#define PLAYER_HEALTH_COLOR_LOW (Color){ 255, 50, 20, 255 }
#define PLAYER_HEALTH_COLOR_HIGH (Color){ 50, 255, 20, 255 }

#define PLAYER_WEAPON_FULLAUTO 0
#define PLAYER_WEAPON_SEMIAUTO 1

#define DISABLE_AIM_WHEN_RELEASED 0
#define DISABLE_AIM_WHEN_MOUSERIGHT 1


struct player_t {

    Camera   cam;
    float cam_yaw;
    float cam_pitch;
    Vector3  position;  // "Read only" change the camera position instead.
    Vector3  prev_position; // Previous frame position.
    Vector3  hitbox_size;
    float    hitbox_y_offset;
    float    height;
    Vector3  looking_at; // normalized vector where the player is looking towards
    int      is_moving;
    Vector3  velocity;

    // Movement related.
    float    speed; // "Read only". Updated from 'input.c'
    float    walkspeed;
    /*TODO*/float walkspeed_aim_mult; // Multiply speed when aiming
    float    run_speed_mult; // Multiply speed when running
    float    air_speed_mult; // Multiply 
    float    ground_friction;
    float    air_friction;
    float    jump_force;
    float    gravity;
    int      onground;
    /*TODO*/int max_jumps;
    /*TODO*/int num_jumps_inair;

    float    dash_timer;
    float    dash_timer_max; // How long to wait until player can use dash?
    float    dash_speed;
    Vector3  dash_velocity;

    int      in_water;

    int      noclip;
    int      is_aiming;
    int      alive;


    int armor;
    int max_armor;
    float armor_damage_dampen;

    // External force may be applied to player.
    Vector3  ext_force_vel;
    Vector3  ext_force_acc;

   
    // If the player has been aiming but not fired any shots: disable aiming.
    float aim_idle_timer;

    // Player can update powerups with XP. 
    // Updated when player kills an enemy.
    int xp; 


    int enable_fov_effect;
    float fovy_change;
    
    // How aiming should be disabled?
    // When player holds <aim button> for X amount of time (see input.c)
    // 'disable_aim_mode' is changed to DISABLE_AIM_MODE_WHEN_RELEASED
    // if 'aim_button_hold_timer'
    // was less than X amount of time time it will be set to DISABLE_AIM_MODE_WHEN_MOUSERIGHT
    int   disable_aim_mode; 
    float aim_button_hold_timer; 


    struct item_t* item_in_crosshair;
   
    struct powerup_shop_t powerup_shop;
    struct inventory_t inventory;

    Model gunmodel;
    Material arms_material;
    Material hands_material;

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

    int kills;

    struct weapon_t  weapon; // Weapon stats.
    //struct psystem_t weapon_psys;
    float accuracy_modifier; // Used to decrease accuracy if shooting rapidly.
    float accuracy_control; // 'WEAPON_ACCURACY_MIN' to 'WEAPON_ACCURACY_MAX'. 
                            //  higher number means better accuracy control
    
    float time_from_last_shot;

    float gunfx_timer;
    Model gunfx_model;

    // Some weapon related variables may be stored else where.
    // For example the firerate because enemies use pointers to "global weapons"
    // and not their own weapon like the player has.
    float firerate_timer;
    float firerate;

    // Entities cant have this setting.
    int weapon_firetype; 

    int any_gui_open;
    
};


void init_player_struct(struct state_t* gst, struct player_t* p);
void delete_player  (struct player_t* p);
void player_respawn (struct state_t* gst, struct player_t* p);
void player_shoot   (struct state_t* gst, struct player_t* p);
void player_damage  (struct state_t* gst, struct player_t* p, float damage);
void player_heal    (struct state_t* gst, struct player_t* p, float heal);
void player_add_xp  (struct state_t* gst, int xp);
void player_apply_force(struct state_t* gst, struct player_t* p, Vector3 force);


/*TODO*/void player_update_death_animation(struct state_t* gst, struct player_t* p);

// TODO: Rename these.
// updates player's variables. not movement for movement see (input.c)
void player_update(struct state_t* gst, struct player_t* p);
void player_render(struct state_t* gst, struct player_t* p);
void player_update_movement(struct state_t* gst, struct player_t* p);
void player_update_camera(struct state_t* gst, struct player_t* p);

// This function can only render in 2D:
void render_player_stats(struct state_t* gst, struct player_t* p);

BoundingBox get_player_boundingbox(struct player_t* p);


#endif
