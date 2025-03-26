#include <stdio.h>

#include "state.h"
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
            NULL, NO_EXTRADATA, NO_IDB
            );
}
/*




void setup_weapon(
        struct state_t* gst,
        struct weapon_t* w,
        struct weapon_t weapon_stats
){

    w->cooling_level = 0.0;
    w->heat_increase = 0.0;
    w->overheat_temp = -1.0;
    *w = weapon_stats;

    //create_psystem(gst, &w->psystem, 512, update_callback_ptr, pinit_callback_ptr);

    w->psystem.particle_mesh = GenMeshSphere(0.356, 8, 8);
    w->psystem.particle_material = LoadMaterialDefault();
    w->psystem.particle_material.shader = gst->shaders[PROJECTILES_PSYSTEM_SHADER];

    w->knockback = CLAMP(w->knockback, 0.0, 10.0);
    w->accuracy = CLAMP(w->accuracy, 0.0, 10.0);
    w->prj_damage = CLAMP(w->prj_damage, 0.0, 10000.0);
    w->temp = 0.0;
}



int weapon_add_projectile(
        struct state_t* gst,
        struct weapon_t* w,
        Vector3 position,
        Vector3 direction,
        float accuracy,
        struct psystem_t* psystem
){
    int result = 0;

    if(w->overheat_temp > 0.0) {
        if(w->temp < w->overheat_temp) {
            w->temp += w->heat_increase;
        }

        if(w->temp >= w->overheat_temp) {
            goto skip;
        }
    }

    const float ak = 0.1;
    const float ac = ak - map(accuracy, WEAPON_ACCURACY_MIN, WEAPON_ACCURACY_MAX, 0.0, ak);

    direction.x += RSEEDRANDOMF(-ac, ac);
    direction.y += RSEEDRANDOMF(-ac, ac);
    direction.z += RSEEDRANDOMF(-ac, ac);


    w->psystem.userptr = w;
    add_particles(gst, psystem, 1, position, direction, NULL, NO_EXTRADATA);

    result = 1;

skip:
    return result;
}



void weapon_update(struct state_t* gst, struct weapon_t* w) {
    update_psystem(gst, &w->psystem);

    if(w->temp > 0.0) {
        w->temp -= gst->dt * w->cooling_level;
    }
    w->temp = CLAMP(w->temp, 0.0, w->overheat_temp);


}


*/

