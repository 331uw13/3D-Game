#ifndef WEAPON_H
#define WEAPON_H

#include <stddef.h>
#include <raylib.h>

#include "psystem.h"



#define WEAPON_ACCURACY_MAX 10.0
#define WEAPON_ACCURACY_MIN 0.0

#define WEAPON_DAMAGE_MAX 500.0

#define PLAYER_WEAPON_GID 0
#define ENEMY_WEAPON_GID 1
#define INVLID_WEAPON_GID 2


#define LQMAG_TYPE_SMALL 0
// ... (adding more later)

#define MAX_LQMAG_TYPES 1



#define LQMAG_CONDITION_PERFECT 0
#define LQMAG_CONDITION_GOOD 1
#define LQMAG_CONDITION_OK 2
#define LQMAG_CONDITION_POOR 3
#define LQMAG_CONDITION_BAD 4
#define LQMAG_CONDITION_BROKEN 5  
#define LQMAG_CONDITION_TRASH 6


// Liquid Magazine. (Container for ammo)
struct lqmag_t {
    int infinite; // Enemies may have infinite magazine.

    float ammo_level;
    float capacity;

    // Liquid magazine "health" may drop
    // if it was hit by projectile or damaged in other ways.
    float condition_value; // 0.0 - 100.0
    int   condition;

    // Liquid magazine may leak  (TODO)
    // after the condition is below 'LQMAG_CONDITION_POOR'
    float leak_value;

    Model* modelptr;
};


struct weapon_t {

    int gid; // "Group id". This can be used to know who the weapon belongs to.

    // Weapon settings.

    float    accuracy;  // 0.0 (low accuracy) - 10.0 (high accuracy)
    float    damage;
    int      critical_chance; // 0% - 100%
    float    critical_mult;
    Color    color; // Projectile color

    struct lqmag_t lqmag;

    // Projectile settings.
    float    prj_speed;
    float    prj_max_lifetime;
    Vector3  prj_hitbox_size;
    float    prj_scale;

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

void update_weapon_lqmag_condition(struct weapon_t* weapon);
void init_weapon_lqmag(struct state_t* gst, struct weapon_t* weapon, int type);

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
