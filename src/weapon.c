#include <stdio.h>

#include "state.h"
#include "weapon.h"
#include "util.h"



float compute_weapon_accuracy(struct state_t* gst, struct weapon_t* weapon) {
    float k = 0.1;
    return k-map(weapon->accuracy, 0, 10, 0.0, k);
}


void setup_weapon(
        struct state_t* gst,
        struct weapon_t* w,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, Vector3,Vector3,void*,int),
        struct weapon_t weapon_stats
){

    *w = weapon_stats;

    create_psystem(gst, &w->psystem, 64, update_callback_ptr, pinit_callback_ptr);

    w->psystem.particle_mesh = GenMeshSphere(0.5, 8, 8);
    w->psystem.particle_material = LoadMaterialDefault();
    w->psystem.particle_material.shader = gst->shaders[PROJECTILES_PSYSTEM_SHADER];

    w->knockback = CLAMP(w->knockback, 0.0, 10.0);
    w->accuracy = CLAMP(w->accuracy, 0.0, 10.0);
    w->prj_damage = CLAMP(w->prj_damage, 0.0, 10000.0);
}

void delete_weapon(struct weapon_t* w) {
    delete_psystem(&w->psystem);
}


void weapon_add_projectile(
        struct state_t* gst,
        struct weapon_t* w,
        Vector3 position,
        Vector3 direction
){

    const float ak = 0.1;
    const float ac = ak - map(w->accuracy, 0.0, 10.0, 0.0, ak);

    direction.x += RSEEDRANDOMF(-ac, ac);
    direction.y += RSEEDRANDOMF(-ac, ac);
    direction.z += RSEEDRANDOMF(-ac, ac);


    w->psystem.userptr = w;
    add_particles(gst, &w->psystem, 1, position, direction, NULL, NO_EXTRADATA);
}



void weapon_update(struct state_t* gst, struct weapon_t* w) {
    update_psystem(gst, &w->psystem);
    // ...
}

void weapon_render_projectiles(struct state_t* gst, struct weapon_t* w) {
    float psystem_color[3] = {
        (float)w->prj_color.r / 255.0,
        (float)w->prj_color.g / 255.0,
        (float)w->prj_color.b / 255.0,
    };

    SetShaderValue(
            gst->shaders[PROJECTILES_PSYSTEM_SHADER], 
            gst->fs_unilocs[PROJECTILES_PSYSTEM_COLOR_FS_UNILOC],
            psystem_color,
            SHADER_UNIFORM_VEC3
            );


    render_psystem(gst, &w->psystem);
}

