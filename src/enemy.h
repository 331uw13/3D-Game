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



// Enemy types.
#define ENEMY_LVL0 0




#define ENEMY_WEAPON_COLOR ((Color){255, 0, 255, 255})
#define ENEMY_MAX_MATRICES 4
#define ENEMY_MAX_HITBOXES 4


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


struct hitbox_t {
    Vector3 size;
    Vector3 offset;
    float damage_mult;
    int id;
};


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
    struct hitbox_t hitboxes[ENEMY_MAX_HITBOXES];
    size_t          num_hitboxes;

    float dist_to_player; // Distance to player.

    // Each enemy has different matrices for different "body parts".
    // The indices are defined in heir own header file.
    Matrix matrix[ENEMY_MAX_MATRICES];


    float target_range; // how far can the enemy "see" the player
    float target_fov;   // (10 - 360)

    int   alive;
    float health;
    float max_health;

    // When enemy gets hit. velocity may be applied.
    Vector3 knockback_velocity;
    
    // How long the enemy is stunned after it was hit.
    float stun_timer;
    float max_stun_time; 
    
    // Used for rotating enemy.
    Quaternion Q_prev;
    Quaternion Q_target;
    float      angle_change;  // How much 'Q_prev' is changed to 'Q_target'.
    Vector3    rotation;      // Matrix rotation.

    // For random angles.
    // Enemy may be "searching" for target and changing rotation.
    Quaternion Q_rnd_prev;
    Quaternion Q_rnd_target;
    float      rnd_angle_change;

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
    float accuracy_modifier;
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

// IMPORTANT NOTE: remember to adjust 'MAX_ENEMY_MODELS' in 'state.h'
// when adding more models!
int load_enemy_model(struct state_t* gst, u32 enemy_type, const char* model_filepath, int texture_id);


struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int mood,
        Model* model,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        float target_range,
        float target_fov,
        float firerate,
        void(*update_callback)(struct state_t*, struct enemy_t*),
        void(*render_callback)(struct state_t*, struct enemy_t*),
        void(*death_callback)(struct state_t*, struct enemy_t*),
        void(*spawn_callback)(struct state_t*, struct enemy_t*),
        void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/)
);

void enemy_add_hitbox(
        struct enemy_t* ent, 
        Vector3 hitbox_size,
        Vector3 hitbox_offset,
        float damage_multiplier
);

// Simpler function to use than 'create_enemy'
void spawn_enemy(
        struct state_t* gst,
        int enemy_type,
        int max_health,
        int mood,
        Vector3 position
);

void delete_enemy(struct state_t* gst, size_t enemy_index);

void update_enemy(struct state_t* gst, struct enemy_t* ent);
void render_enemy(struct state_t* gst, struct enemy_t* ent);
void enemy_death(struct state_t* gst, struct enemy_t* ent);
void enemy_hit(
        struct state_t* gst,
        struct enemy_t* ent,
        struct weapon_t* weapon,
        float damage_mult,
        Vector3 hit_position,
        Vector3 hit_direction
);


// Check target range and is terrain blocking view
int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent);

// IMPORTANT NOTE: The body matrix must be set correctly before calling this function.
//  It is used to calculate the cross product.
//  see 'enemies/enemy_lvl0.c' for example.
int player_in_enemy_fov(struct state_t* gst, struct enemy_t* ent, Matrix* body_matrix);

// Returns pointer to the hitbox that was collided with 'boundingbox'
// or NULL if no collision.
struct hitbox_t* check_collision_hitboxes(BoundingBox* boundingbox, struct enemy_t* ent);




#endif
