#ifndef WEAPON_H
#define WEAPON_H

#include <stddef.h>
#include <raylib.h>

#include "light.h"
#include "psystem.h"

#define MAX_WEAPON_PROJECTILES 64


// TODO: render projectiles as particle system.



// TODO: create array of weapons in state struct and entities can use the weapon from there.

struct weapon_t {
    struct psystem_t psystem; // Use particle system for projectiles.

    // Weapon settings.

    float    knockback;
    float    accuracy;  // 0.0 (low accuracy) - 10.0 (high accuracy)

    // Projectile settings.
    
    float    prj_speed;
    float    prj_damage;
    float    prj_max_lifetime;
    Vector3  prj_size;
    Color    prj_color;
};



void setup_weapon(
        struct state_t* gst,
        struct weapon_t* w,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, Vector3,Vector3,void*,int),
        struct weapon_t weapon_stats
        );

void delete_weapon(struct weapon_t* w);


float compute_weapon_accuracy(struct state_t* gst, struct weapon_t* weapon);

void weapon_add_projectile(
        struct state_t* gst,
        struct weapon_t* w,
        Vector3 position,
        Vector3 direction
        );

// Update and render projectiles.
void weapon_update(
        struct state_t* gst,
        struct weapon_t* w
        );

#endif
