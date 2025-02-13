#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>


struct state_t;
struct psystem_t;
struct particle_t;
struct projectile_t;

#define ENEMY_RND_SEARCH_RADIUS 10.0
#define ENEMY_RND_SEARCH_MIN_RADIUS 3.0

// enemy state
#define ENEMY_TURNING 1
#define ENEMY_SEARCH 2

struct enemy_t {    
    long int id;
    int health;
    int max_health;
    Vector3 position;
    Vector3 hitbox;

    Model model;
    int model_loaded;

    float angle_change;
    float forward_angle;
    float previous_angle;

    // when enemy gets hit. velocity is applied.
    Vector3 knockback_velocity;
    Vector3 hit_direction;
    Vector3 rotation_from_hit;

    float hit_force_friction;
    int was_hit;

    int dest_reached;
    Vector3 travel_dest;
    Vector3 travel_start; 
    float travelled;

    int state;
};

#define ENEMY_0_MAX_HEALTH 100


void free_enemyarray(struct state_t* gst);

struct enemy_t* create_enemy(
        struct state_t* gst, 
        const char* model_filepath,
        int texture_id,
        int max_health,
        Vector3 hitbox,
        Vector3 position
        );

void draw_enemy_hitbox(struct enemy_t* enemy);
void move_enemy(struct enemy_t* enemy, Vector3 position);

void update_enemy(struct state_t* gst, struct enemy_t* enemy);
void enemy_hit(struct state_t* gst, struct enemy_t* enemy, struct projectile_t* proj);

void enemy_hit_psys_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* p
        );
void enemy_hit_psys_pinit(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* p,
        void* extradata_ptr,
        int has_extradata
        );

#endif
