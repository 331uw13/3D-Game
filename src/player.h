#ifndef PLAYER_H
#define PLAYER_H


#include "weapon.h"
#include "light.h"
#include "inventory.h"
#include "item.h"
#include "enemy.h"

struct state_t;


#define ABS_MAX_ARMOR 20
#define ABS_MAX_HEALTH 1000

#define MAX_DEFAULT_HEALTH 200
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

#define MAX_PRJMOD_INDICES 64

#define MOVEMENT_STATE_STANDING 0
#define MOVEMENT_STATE_SNEAKING 1
#define MOVEMENT_STATE_WALKING 2
#define MOVEMENT_STATE_RUNNING 3


struct player_t {
    int render;

    Camera   cam;
    Vector3  cam_forward;

    struct inventory_t inventory;

    float cam_yaw;
    float cam_pitch;
    Vector3  position;  // "Read only" change the camera position instead.
    Vector3  prev_position; // Previous frame position.
    Vector3  hitbox_size;
    float    hitbox_y_offset;
    float    height;
    Vector3  looking_at; // Normalized vector where the player is looking towards
    Vector3  velocity;
    Vector3  spawn_point;
    int      is_moving;
    int      is_aiming;

    int      movement_state;

    struct biome_t* current_biome;

    // Movement related.
    float    speed; // "Read only". Updated from 'input.c'
    float    walkspeed;
    float    walkspeed_aim_mult; // Multiply speed when aiming
    float    run_speed_mult; // Multiply speed when running
    float    air_speed_mult; // Multiply 
    float    ground_friction;
    float    air_friction;
    float    jump_force;
    float    gravity;
    int      onground;

    float    ccheck_radius; // How close will collision checks start to happen?
    struct hitbox_t hitboxes[MAX_HITBOXES]; 
    // NOTE: 'num_hitboxes' is not needed for player.

    float    dash_timer;
    float    dash_timer_max; // How long to wait until player can use dash?
    float    dash_speed;
    Vector3  dash_velocity;

    int inspecting_weapon;
    float weapon_inspect_interp;

    float weapon_offset_interp;
    Matrix last_weapon_matrix;

    int in_scope_view;
    
    // Pointers to items in inventory.
    struct item_t* item_in_hands;
    struct item_t* item_to_change;
    float item_change_timer;
    int   changing_item;


    int noclip;
    int alive;

    float armor;
    float max_armor;
    float armor_damage_dampen;
    
    int wants_to_pickup_item;


    // External force may be applied to player.
    Vector3  ext_force_vel;
    Vector3  ext_force_acc;

    Vector2 cam_random_dir; // For recoil.

    float accuracy_modifier; // This changes based on player movement.

    Model gunfx_model;
    float gunfx_timer;
   
    // Player can update powerups with XP. 
    // Updated when player kills an enemy.
    int xp; 

    float health;
    float max_health;

    int kills[MAX_ENEMY_TYPES];

    int any_gui_open;

};



void init_player_struct(struct state_t* gst, struct player_t* p);
void delete_player  (struct state_t* gst, struct player_t* p);
void player_respawn (struct state_t* gst, struct player_t* p);
void player_shoot   (struct state_t* gst, struct player_t* p);
void player_damage  (struct state_t* gst, struct player_t* p, float damage);
void player_heal    (struct state_t* gst, struct player_t* p, float heal);
void player_add_xp  (struct state_t* gst, int xp);
void player_apply_force(struct state_t* gst, struct player_t* p, Vector3 force);
int  point_in_player_view(struct state_t* gst, struct player_t* p, Vector3 point, float fov_range);

/*TODO*/void player_update_death_animation(struct state_t* gst, struct player_t* p);

// Returns '-1' If not in biome shift area.
// Returns 0 - 'MAX_BIOME_TYPES' When in area.
// Example: if returned value is 1, 
//          we can know fog and other things must start to blend from
//          BIOMEID_HAZY <-> BIOMEID_EVIL.
int  playerin_biomeshift_area(struct state_t* gst, struct player_t* p);

// Change currently holding item.
void player_change_holding_item(struct state_t* gst, struct player_t* p, struct item_t* item);
void player_set_scope_view(struct state_t* gst, struct player_t* p, int view_enabled);

// TODO: Rename these.
void player_update(struct state_t* gst, struct player_t* p);
void render_player(struct state_t* gst, struct player_t* p);
void render_player_gunfx(struct state_t* gst, struct player_t* p);
void player_update_movement(struct state_t* gst, struct player_t* p);
void player_update_camera(struct state_t* gst, struct player_t* p);

// This function can only render in 2D:
void render_player_stats(struct state_t* gst, struct player_t* p);

// TODO: This may not be needed anymore ????

BoundingBox get_player_boundingbox(struct player_t* p);


#endif
