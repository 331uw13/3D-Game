#ifndef WEAPON_H
#define WEAPON_H

#include <stddef.h>
#include <raylib.h>

#include "light.h"
#include "psystem.h"

#define MAX_WEAPON_PROJECTILES 512


#define WEAPON_ACCURACY_MAX 10.0
#define WEAPON_ACCURACY_MIN 0.0


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


    // Weapon temperature:
    // 'temp' increases when shooting. Cooldown is applied if the weapon overheats
    // It also cools down slowly over time.
    // by setting 'overheat_temp' to negative value ignores this effect (set by default).
    // 'cooling_level' is how fast the weapon cools down.
    float    temp;
    float    heat_increase;
    float    overheat_temp;
    float    cooling_level;

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

int weapon_add_projectile(
        struct state_t* gst,
        struct weapon_t* w,
        Vector3 position,
        Vector3 direction,
        float accuracy
        );

void weapon_update(
        struct state_t* gst,
        struct weapon_t* w
        );

void weapon_render_projectiles(
        struct state_t* gst,
        struct weapon_t* w
        );

#endif
