#ifndef WEAPON_H
#define WEAPON_H

#include <stddef.h>
#include <raylib.h>

#include "psystem.h"



#define WEAPON_ACCURACY_MAX 10.0
#define WEAPON_ACCURACY_MIN 0.0

#define PLAYER_WEAPON_ID 0
#define ENEMY_WEAPON_ID 1
#define INVLID_WEAPON_ID 2

struct weapon_t {

    int id; // This can be used to know who the weapon belongs to.

    // Weapon settings.

    float    accuracy;  // 0.0 (low accuracy) - 10.0 (high accuracy)
    float    damage;
    int      critical_chance; // 0% - 100%
    float    critical_mult;
    Color    color;
    

    // Projectile settings.
    
    float    prj_speed;
    float    prj_max_lifetime;
    Vector3  prj_hitbox_size;


    // Weapon temperature: (TODO)
    // 'temp' increases when shooting. Cooldown is applied if the weapon overheats
    // It also cools down slowly over time.
    // by setting 'overheat_temp' to negative value ignores this effect (set by default).
    // 'cooling_level' is how fast the weapon cools down.
    float    temp;
    float    heat_increase;
    float    overheat_temp;
    float    cooling_level;

};

struct state_t;
struct psystem_t;


#define NO_ACCURACY_MOD 0.0

float get_weapon_damage(struct weapon_t* weapon);
void add_projectile(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct weapon_t* w,
        Vector3 initial_pos,
        Vector3 direciton,
        float accuracy_modifier // Used to decrease accuracy if/when shooting rapidly.
        );

#endif
