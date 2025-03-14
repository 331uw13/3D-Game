#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>
#include <stddef.h>

#include "typedefs.h"

#define ENT_STATE_IDLE 0
#define ENT_STATE_SEARCHING_TARGET 1
#define ENT_STATE_HAS_TARGET 2
#define ENT_STATE_CHANGING_ANGLE 3
#define ENT_STATE_WASHIT 4

#define ENT_TRAVELING_DISABLED 0
#define ENT_TRAVELING_ENABLED 1

// "mood" for enemy.
#define ENT_FRIENDLY 0
#define ENT_HOSTILE 1



// Spawn settings.
/*
#define SPAWN_RADIUS 2000

#define MAX_ENEMY_LVL0  50
#define ENEMY_TYPE_SPAWNTIME 5 // How long to wait until more can be spawned? (in seconds)
*/


// Enemy types.

#define ENEMY_LVL0 0
#define MAX_ALL_ENEMIES 32 // Total max enemies.
// ...


// Misc.
#define ENEMY_WEAPON_COLOR ((Color){255, 0, 255, 255})
#define ENEMY_MAX_MATRICES 4




// This handles all basic behaviour for enemies.
// Then calls 'enemies/enemy_lvl*.c' (depending on "enemy type") to handle the rest.

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
    Model* modelptr;
    int type;
    int enabled;

    /*
    // When enemy dies the model "breaks"
    Model broken_model;
    Matrix* broken_matrices;
    Vector3* broken_mesh_velocities; // Velocities for "broken" meshes.
    Vector3* broken_mesh_rotations;  // Rotations for "broken" meshes.
    */


    Vector3 position; // <- NOTE: "read only". modify the model's transform instead.
    Vector3 hitbox_size; // TODO: multiple hitboxes.
    Vector3 hitbox_position; // hitbox position from 'enemy position'.

    // Each enemy has different matrices for different "body parts".
    // The indices are defined in heir own header file.
    Matrix matrix[ENEMY_MAX_MATRICES];


    float target_range; // how far can the enemy "see" the player

    int   alive;
    float health;
    float max_health;

    /*
    int   broken_model_despawned;
    float broken_model_despawn_maxtime;
    float broken_model_despawn_timer;
    */

    // When enemy gets hit. velocity is applied.
    Vector3 knockback_velocity;
    
    // How long the enemy is stunned after it was hit.
    float stun_timer;
    float max_stun_time; 
    
    // Used for rotating enemy.
    Quaternion Q0;
    Quaternion Q1;
    float      angle_change; // how much angle is changed to another. 0.0 to 1.0
    float      forward_angle;

    // For any kind of movement enemy has.
    struct enemy_travel_t travel;

    // Used for selecting new point where to travel
    // when state is 'SEARCHING_TARGET'
    float rnd_search_radius;
    float rnd_search_min_radius;

    int state;
    int previous_state;

    struct weapon_t*   weaponptr;
    struct psystem_t*  weapon_psysptr;
    float firerate;
    float firerate_timer;
    int gun_index; // Switch between model's guns.

    int mood;
    int has_target;

    size_t index; // Index in gst->enemies array.
    
    // Callbacks

    void(*update_callback)(struct state_t*, struct enemy_t*);
    void(*render_callback)(struct state_t*, struct enemy_t*);
    void(*death_callback)(struct state_t*, struct enemy_t*);
    void(*spawn_callback)(struct state_t*, struct enemy_t*);
    void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/);

};


struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int mood,
        Model* model,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size,
        Vector3 hitbox_position,
        float target_range,
        float firerate,
        void(*update_callback)(struct state_t*, struct enemy_t*),
        void(*render_callback)(struct state_t*, struct enemy_t*),
        void(*death_callback)(struct state_t*, struct enemy_t*),
        void(*spawn_callback)(struct state_t*, struct enemy_t*),
        void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/)
);


// Simpler function to use than 'create_enemy'
void spawn_enemy(
        struct state_t* gst,
        int enemy_type,
        int max_health,
        int mood,
        Vector3 position
);

/*
void spawn_enemy(struct state_t* gst, int enemy_type);
void spawn_enemies(struct state_t* gst, int enemy_type, int count, int max_count);
*/

//void load_enemy_broken_model(struct enemy_t* ent, const char* broken_model_filepath);

// TODO: rename to 'disable_enemy'
//void delete_enemy(struct enemy_t* ent);


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

int load_enemy_model(struct state_t* gst, u32 enemy_type, const char* model_filepath, int texture_id);


#endif
