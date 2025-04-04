#ifndef PRJ_MOD_H
#define PRJ_MOD_H

#include <stddef.h>
#include <raylib.h>


// Some powerups may change player's projectile behaviour
// projectile modifiers are stored in 'struct player_t'

struct state_t;
struct psystem_t;
struct particle_t;
struct enemy_t;

// "Return flag" should be returned from functions
// for 'weapon_psys_prj_update' to know what to do next.
#define PRJMOD_RF_NONE 0       // Projectile is disabled after it hit something.
#define PRJMOD_RF_NODISABLE 1  // Projectile is not disabled after it hit something.
// (more can be added here if needed..)

// NOTE:
//  When calling 'add_prjmod()', it will return index where it was created
//  and adds the index into 'player.prjmod_indices' array.

// 'prjmod_id':
#define PRJMOD_TEST_ID 0
#define PRJMOD_FMJ_ABILITY_ID 1
#define PRJMOD_GRAVITY_PRJ_ID 2


struct prjmod_t {
    
    // Set to positive number if you want to handle damage to enemies by yourself
    int cancel_damage;

    // NOTE: Function pointers may be set to NULL if they are not used.

    int(*enemy_hit_callback)( // (Called after enemy hit was handled).
            struct state_t*,
            struct psystem_t*,  // Projectile's particle system.
            struct particle_t*, // Current projectile.
            struct enemy_t*
            );

    // Environment hit.
    int(*env_hit_callback)(
            struct state_t*,
            struct psystem_t*,
            struct particle_t*,
            Vector3  // Surface normal, Hit position equals to particle position.
            );

    // Called once when projectile is initialized.
    int(*init_callback)(
            struct state_t*,
            struct psystem_t*,
            struct particle_t*
            );

    // Called every frame when projectile is updated.
    int(*update_callback)(
            struct state_t*,
            struct psystem_t*,
            struct particle_t*
            );

    size_t  index;  // Index in 'player.prjmods' array
};

struct prjmod_index_t {
    long int unique_id;
    size_t   index;
};


// Returns the index where the prjmod was added in 'player.prjmods' array
size_t add_prjmod(struct state_t* gst, struct prjmod_t* prjmod, size_t prjmod_id);
void   rem_prjmod(struct state_t* gst, size_t prjmod_id);
void   delete_prjmods(struct state_t* gst);

void   call_prjmods_update    (struct state_t* gst, struct psystem_t* psys, struct particle_t* part);
void   call_prjmods_init      (struct state_t* gst, struct psystem_t* psys, struct particle_t* part);
void   call_prjmods_enemy_hit (struct state_t* gst, struct psystem_t* psys, struct particle_t* part, struct enemy_t* ent);
void   call_prjmods_env_hit   (struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal);

#endif

