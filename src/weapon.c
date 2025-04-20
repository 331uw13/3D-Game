#include <stdio.h>

#include "state/state.h"
#include "weapon.h"
#include "util.h"


float get_weapon_damage(struct weapon_t* weapon) {
    float damage = weapon->damage;

    int critical_hit = (GetRandomValue(0, 100) < weapon->critical_chance);
    if(critical_hit) {
        damage *= weapon->critical_mult;
    }

    return damage;
}

void add_projectile(struct state_t* gst, struct psystem_t* psys, struct weapon_t* w,
        Vector3 initial_pos, Vector3 direction, float accuracy_modifier) {
    
    if(!w->lqmag.infinite) {
        if(w->lqmag.ammo_level <= 0.0) {
            return;
        }

        w->lqmag.ammo_level -= 1.0;
        printf("%f\n", w->lqmag.ammo_level);
    }
    // Modify direction based on accuracy.
    float k = 0.1;
    float a = k-map(w->accuracy - accuracy_modifier, 0, 10, 0.0, k);

    direction = (Vector3) {
        direction.x + RSEEDRANDOMF(-a, a),
        direction.y + RSEEDRANDOMF(-a, a),
        direction.z + RSEEDRANDOMF(-a, a)
    };

    add_particles(
            gst,
            psys,
            1,
            initial_pos,
            direction,
            (Color){0},
            w, HAS_EXTRADATA, NO_IDB
            );
}

void update_weapon_lqmag_condition(struct weapon_t* weapon) {
    weapon->lqmag.condition = LQMAG_CONDITION_PERFECT;
    weapon->lqmag.leak_value = 0.0;
}

void init_weapon_lqmag(struct state_t* gst, struct weapon_t* weapon, int type) {
    
    weapon->lqmag.infinite = 0;
    weapon->lqmag.capacity = 300.0;
    weapon->lqmag.ammo_level = weapon->lqmag.capacity;

    weapon->lqmag.condition_value = 100.0;
    update_weapon_lqmag_condition(weapon);

}


