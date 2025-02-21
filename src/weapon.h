#ifndef WEAPON_H
#define WEAPON_H

#include <stddef.h>
#include <raylib.h>

#include "light.h"

#define MAX_WEAPON_PROJECTILES 64


struct projectile_t {
    Vector3 position;
    Vector3 direction;

    float lifetime;
    int alive;

    struct light_t light;
};


struct weapon_t {
    
    struct projectile_t projectiles[MAX_WEAPON_PROJECTILES];

    long int  num_alive_projectiles;
    size_t    prj_nextindex;

    // Weapon settings.

    float    knockback;
    float    accuracy;  // 0.0 (low accuracy) - 10.0 (high accuracy)

    // Projectile settings.
    
    float    prj_speed;
    float    prj_damage;
    float    prj_max_lifetime;
    Vector3  prj_size;

};

float compute_weapon_accuracy(struct state_t* gst, struct weapon_t* weapon);

void weapon_add_projectile(
        struct state_t* gst,
        struct weapon_t* w,
        Vector3 position,
        Vector3 direction
        );

// Update and render projectiles.
void weapon_update_projectiles(
        struct state_t* gst,
        struct weapon_t* w
        );

#endif
