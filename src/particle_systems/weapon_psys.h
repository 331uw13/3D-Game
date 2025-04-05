#ifndef WEAPON_PSYSTEM_H
#define WEAPON_PSYSTEM_H


#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;

// Projectile update and initialization for any type of normal weapon.


// WEAPON PROJECTILE PARTICLE UPDATE
void weapon_psys_prj_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);

// WEAPON PROJECTILE PARTICLE INITIALIZATION
void weapon_psys_prj_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
);

// For player's powerups.
// TODO: comment better.

#define PWRUP_ENT_PASSED_I               0 // For: 'particle->user_i' Used by 'POWERUP_FMJPRJ_ABILITY'
#define PWRUP_PRJ_HAS_GRAVITY_TARGET_I   1 // For: 'partilce->user_i' Used by 'POWERUP_GRAVITY_PROJECTILES'
#define PWRUP_PRJ_GRAVITY_TARGET_I       2 // For: 'particle->user_i' Used by 'POWERUP_GRAVITY_PROJECTILES'
#define PWRUP_PRJ_CLOUDBURST_TIMER_I     0 // For: 'particle->user_f' Used by 'POWERUP_CLOUDBURST'


#endif
