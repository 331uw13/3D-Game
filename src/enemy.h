#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>
#include <stddef.h>

#define ENT_STATE_IDLE 0
#define ENT_STATE_SEARCHING_TARGET 1
#define ENT_STATE_HAS_TARGET 2
#define ENT_STATE_CHANGING_ANGLE 3
#define ENT_STATE_WASHIT 4

#define ENT_TRAVELING_DISABLED 0
#define ENT_TRAVELING_ENABLED 1

#define ENEMY_TYPE_LVL0 0
// ...

#define ENEMY_WEAPON_COLOR ((Color){255, 0, 255, 255})


#define ENEMY_NUM_BROKEN_MESH 4

// This handles all basic behaviour for enemies.
// Then calls 'enemies/enemy_lvl*.c'(depending on "enemy type") 
// to handle the rest if needed

#include "weapon.h"

struct state_t;


struct enemy_travel_t {
    Vector3 start; // Start position
    Vector3 dest;  // Destination position.
    float travelled; // Used for linear interpolation.

    int dest_reached;
    int enabled; // Some enemies may not move.
};

// TODO: remove rotation from hit!

struct enemy_t {
    Model model;

    // When enemy dies the model "breaks"
    Model broken_model;
    Matrix* broken_matrices;
    Vector3* broken_mesh_velocities; // Velocities for "broken" meshes.
    Vector3* broken_mesh_rotations;  // Rotations for "broken" meshes.

    int type;

    Vector3 position; // <- NOTE: "read only". modify the model's transform instead.
    Vector3 hitbox_size; // TODO: multiple hitboxes.
    Vector3 hitbox_position; // hitbox position from 'enemy position'.
    Matrix body_matrix; // Some enemies have rotating body and 'model.transform' is used for legs etc.


    float target_range; // how far can the enemy "see" the player
    int   has_target;

    int   alive;
    float health;
    float max_health;

    int   broken_model_despawned;
    float broken_model_despawn_maxtime;
    float broken_model_despawn_timer;

    // When enemy gets hit. velocity is applied.
    Vector3 knockback_velocity;
    Vector3 hit_direction;
    
    // Rotation may be applied when hit.
    Vector3 rotation_from_hit;

    // How long the enemy is stunned after it was hit.
    float stun_timer;
    float max_stun_time; 
    
    // Used for rotating angles.
    Quaternion Q0;
    Quaternion Q1;
    float      angle_change; // how much angle is changed to another. 0.0 to 1.0

    float forward_angle;

    // For any kind of movement enemy has.
    struct enemy_travel_t travel;

    // Used for selecting new point where to travel
    // when state is 'SEARCHING_TARGET'
    float rnd_search_radius;
    float rnd_search_min_radius;

    int state;
    int previous_state;
    size_t index; // index in gst->enemies array.

    struct weapon_t*   weaponptr;
    struct psystem_t*  weapon_psysptr;

    float firerate;
    float firerate_timer;
    int gun_index; // switch between model's guns.

};

// Probably not going to have ALOT of enemies at once.
// So just leaving the element not used at its index
// And when updating all enemies, looping through the whole array
// And updating only alive ones will be faster than constanty shifting the array back and forth.


struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int texture_id,
        const char* model_filepath,
        const char* broken_model_filepath, // Can be NULL.
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size,
        Vector3 hitbox_position,
        float target_range,
        float firerate
);

void load_enemy_broken_model(struct enemy_t* ent, const char* broken_model_filepath);
// just unloads the model and sets health to 0
void delete_enemy(struct enemy_t* ent);

// "Render settings"
#define ENT_UPDATE_ONLY 0
#define ENT_RENDER_ON_UPDATE 1


// These functions "redirects" the call based on enemy type

void update_enemy(struct state_t* gst, struct enemy_t* ent);
void render_enemy(struct state_t* gst, struct enemy_t* ent);
void enemy_death(struct state_t* gst, struct enemy_t* ent);

void enemy_hit(
        struct state_t* gst,
        struct enemy_t* ent,
        struct weapon_t* weapon, 
        Vector3 hit_position,
        Vector3 hit_direction
        );

int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent);

BoundingBox   get_enemy_boundingbox(struct enemy_t* ent);
void          update_enemy_broken_matrices(struct state_t* gst, struct enemy_t* ent);




#endif
