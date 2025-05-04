#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>
#include <stddef.h>

#include "typedefs.h"

#define ENT_STATE_IDLE 0
#define ENT_STATE_SEARCHING_TARGET 1
#define ENT_STATE_HAS_TARGET 2
#define ENT_STATE_CHANGING_ANGLE 3

#define ENT_TRAVELING_DISABLED 0
#define ENT_TRAVELING_ENABLED 1

// "mood" for enemy.
#define ENT_FRIENDLY 0
#define ENT_HOSTILE 1

// Enemy types.
#define ENEMY_LVL0 0
#define ENEMY_LVL1 1
#define MAX_ENEMY_TYPES 2


#define ENEMY_DESPAWN_RADIUS 1000.0
#define ENEMY_DESPAWN_TIME 60 // (in seconds)


#define ENEMY_LVL0_MAX_HEALTH 100
#define ENEMY_LVL1_MAX_HEALTH 60

#define ENEMY_WEAPON_COLOR ((Color){255, 0, 255, 255})
#define ENEMY_MAX_MATRICES 4
#define ENEMY_MAX_HITBOXES 4

#define ENEMY_DEATH_EXPLOSION_FORCE 15.0
#define ENEMY_DEATH_EXPLOSION_DAMAGE 100.0

// Enemies cant spawn too close to the player.
#define ENEMY_SPAWN_SAFE_RADIUS 600.0 

// Hitbox tag
#define HITBOX_LEGS 0
#define HITBOX_BODY 1
#define HITBOX_HEAD 2

#define ENEMY_XP_GAIN_MAX_BONUS 10

#define ENEMY_LVL0_WEAPON 0
#define ENEMY_LVL1_WEAPON 1
#define MAX_ENEMY_WEAPONS 2
#define MAX_ALL_ENEMIES 64 // Total max enemies.
#define MAX_ENEMY_MODELS 2

// This handles all basic behaviour for enemies.
// Then calls 'enemies/enemy_lvl*.c ...' (depending on "enemy type") to handle the rest.

#include "weapon.h"

struct state_t;


struct enemy_travel_t {
    Vector3 start; // Start position
    Vector3 dest;  // Destination position.
    float travelled; // Used for linear interpolation.

    float time_to_dest;
    int dest_reached;
};


struct hitbox_t {
    Vector3 size;
    Vector3 offset;
    float   damage_mult;
    int     hits;
    int     tag; // Which part does this hitbox belong to?
};


struct enemy_t {
    Model* modelptr;
    int type;
    int enabled;
   
    Vector3 position; // <- NOTE: "read only". modify the model's transform instead.
    struct hitbox_t hitboxes[ENEMY_MAX_HITBOXES];
    size_t          num_hitboxes;

    float dist_to_player; // Distance to player is updated every frame when enemy gets updated.

    // Each enemy has different matrices for different "body parts".
    // The indices are defined in heir own header file.
    Matrix matrix[ENEMY_MAX_MATRICES];


    float target_range; // How far can the enemy "see" the player
    float target_fov;   // (0.0 - 180.0)

    int   alive;
    float health;
    float max_health;

    float despawn_timer;

    // When enemy gets hit. velocity may be applied.
    Vector3 knockback_velocity;
    
    float time_from_hit;

    int xp_gain; // How much xp the player gains when killing this enemy?

    // Used for rotating enemy.
    Quaternion Q_now;
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

    float time_from_target_found;
    float time_from_target_lost;

    int was_hit;

    void(*update_callback)(struct state_t*, struct enemy_t*);
    void(*render_callback)(struct state_t*, struct enemy_t*);
    void(*death_callback)(struct state_t*, struct enemy_t*);
    void(*spawn_callback)(struct state_t*, struct enemy_t*);
    void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/, float/*knockback*/);

};


#define ENEMY_LVL0_MAX_DIFFICULTY 10
#define ENEMY_LVL1_MAX_DIFFICULTY 10


struct ent_spawnsys_t {
    int enemy_type;  
    int difficulty;
    int max_difficulty;

    int can_spawn;
    int max_in_world;         // How many this type of enemy can be in the world at once?
    int max_in_spawn_radius;  // How many can spawn in the same radius?
    float spawn_radius;

    float spawn_timer;
    float spawn_delay; // How long to wait until more may be spawned?
    float add_time_when_killed; // How many seconds to add into 'spawn_timer' when enemy gets killed?

    int num_spawns_min; // Min number of enemies to spawn.
    int num_spawns_max; // Max number of enemies to spawn.

    int to_nextlevel_kills;  // How many kills does it take to increase difficuly?
    int nextlevel_kills_add; // How many to add into 'to_nextlevel_kills' when difficulty is increased?

    // When enough of this type of enemy is killed 'next_spawnsys' is able to spawn.
    // If set to negative this will be ignored.
    int next_spawnsys; 
    int kills_to_next_spawnsys;
};


// IMPORTANT NOTE: remember to adjust 'MAX_ENEMY_MODELS' in 'state.h'
// when adding more models!
int load_enemy_model(struct state_t* gst, u32 enemy_type, const char* model_filepath, int texture_id);


struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int mood,
        Model* model,
        struct psystem_t*  weapon_psysptr,
        struct weapon_t*   weaponptr,
        int     max_health,
        Vector3 initial_position,
        int    xp_gain,
        float  target_range,
        float  target_fov,
        float  firerate,
        void(*update_callback)(struct state_t*, struct enemy_t*),
        void(*render_callback)(struct state_t*, struct enemy_t*),
        void(*death_callback)(struct state_t*, struct enemy_t*),
        void(*spawn_callback)(struct state_t*, struct enemy_t*),
        void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/,float/*knockback*/)
);


void enemy_add_hitbox(
        struct enemy_t* ent, 
        Vector3 hitbox_size,
        Vector3 hitbox_offset,
        float damage_multiplier,
        int hitbox_tag
);

// Simpler function to use than 'create_enemy'
void spawn_enemy(
        struct state_t* gst,
        int enemy_type,
        int mood,
        Vector3 position
);

void update_enemy(struct state_t* gst, struct enemy_t* ent);
void render_enemy(struct state_t* gst, struct enemy_t* ent);
void enemy_death(struct state_t* gst, struct enemy_t* ent);
void enemy_damage(
        struct state_t* gst,
        struct enemy_t* ent,
        float damage,
        struct hitbox_t* hitbox,
        Vector3 hit_position,
        Vector3 hit_direction,
        float knockback
);

void enemy_drop_random_item(struct state_t* gst, struct enemy_t* ent);

int num_enemies_in_radius(struct state_t* gst, int enemy_type, float radius, 
        int* num_in_world/* report back how many in total? (can be NULL)*/);

// Spawns 'n' number of hostile enemies around player with radius of 'spawn_radius'
void spawn_enemies(struct state_t* gst, int enemy_type, size_t n, float spawn_radius); 

void update_enemy_spawn_systems(struct state_t* gst);
void increase_spawnsys_difficulty(struct state_t* gst, struct ent_spawnsys_t* spawnsys);


void setup_default_enemy_spawn_settings(struct state_t* gst);

// Check target range and is terrain blocking view
int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent);

// IMPORTANT NOTE: The body matrix must be set correctly before calling this function.
//  It is used to calculate the cross product.
//  see 'enemies/enemy_lvl1.c' for example.
int player_in_enemy_fov(struct state_t* gst, struct enemy_t* ent, Matrix* body_matrix);

// Check if player is in enemy's fov and the terrain is not blocking the view to player.
// Also if the enemy gets hit this function ignores the enemy fov.
// Function pointers can be NULL.
int enemy_has_target(
        struct state_t* gst, struct enemy_t* ent, Matrix* ent_body_matrix,
        void(*target_found) (struct state_t*, struct enemy_t*),
        void(*target_lost)   (struct state_t*, struct enemy_t*)
        );

// Returns pointer to the hitbox that was collided with 'boundingbox'
// or NULL if no collision.
struct hitbox_t* check_collision_hitboxes(BoundingBox* boundingbox, struct enemy_t* ent);




#endif
