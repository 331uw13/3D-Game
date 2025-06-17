#include <stdio.h>
#include <raymath.h>


#include "berry_collect_psys.h"
#include "../fractalgen.h"
#include "../state/state.h"



// PARTICLE UPDATE
void berry_collect_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

   
    Vector3 target = (Vector3) {
        gst->player.position.x,
        gst->player.position.y - 2.0,
        gst->player.position.z
    };

    if(gst->player.item_in_hands) {
        target = (Vector3) {
            gst->player.item_in_hands->last_pview_transform.m12,
            gst->player.item_in_hands->last_pview_transform.m13 + 1.5,
            gst->player.item_in_hands->last_pview_transform.m14
        };
    }
    


    Vector3 dir = Vector3Subtract(target, part->position);
    dir = Vector3Normalize(dir);
    dir = Vector3Scale(dir, 40.0);
    part->velocity = dir;

    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));
    *part->transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);

    if(Vector3Distance(part->position, target) < 1.0) {
        disable_particle(gst, part);
    }
}

// PARTICLE INITIALIZATION
void berry_collect_psys_init(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    part->max_lifetime = 1.5;
    if(!has_extradata) {
        fprintf(stderr, "\033[31m(ERROR): '%s': Extradata pointer (fractal) is missing!\033[0m\n",
                __func__);
        return;
    }

    struct fractal_t* fractal = 
        (struct fractal_t*)extradata; 
    
    if(!fractal) {
        return;
    }
    if(fractal->num_berries <= 0) {
        return;
    }

    part->color = fractal->berry_color;

    Vector3 fractal_base = (Vector3){
        fractal->transform.m12,
        fractal->transform.m13,
        fractal->transform.m14
    };

    struct berry_t* berry = &fractal->berries[GetRandomValue(0, fractal->num_berries-1)];

    if(berry->level <= 0.01) {
        return;
    }

    part->position = (Vector3) {
        berry->position.x * fractal->scale.x + fractal_base.x,
        berry->position.y * fractal->scale.y + fractal_base.y,
        berry->position.z * fractal->scale.z + fractal_base.z
    };

    float rad = 1.0;
    part->position.x += RSEEDRANDOMF(-rad, rad);
    part->position.y += RSEEDRANDOMF(-rad, rad);
    part->position.z += RSEEDRANDOMF(-rad, rad);

    part->lifetime = 0;
}

