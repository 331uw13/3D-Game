#include "cloud_psys.h"
#include "../state/state.h"
#include "../util.h"

#include "../perlin_noise.h"

#include <raymath.h>
#include <stdio.h>

#define RADIUS 7000

// PARTICLE UPDATE
void cloud_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    float t = normalize(CLAMP(part->lifetime*1.55, 0.0, 1.0), 0.0, 1.0);
    float scale = lerp(t, 0.0, part->scale);

    part->position.y += sin(gst->time*5 + part->index) * 0.01;
    part->position.z += cos(gst->time*5 + part->index) * 0.01;

    Vector3 velocity = Vector3Multiply(gst->weather.wind_dir, part->velocity);
    velocity = Vector3Scale(velocity, gst->weather.wind_strength);
    part->position = Vector3Add(part->position, Vector3Scale(velocity, gst->dt));

    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);   
    Matrix scale_matrix = MatrixScale(scale, scale, scale);

    *part->transform = MatrixMultiply(scale_matrix, translation);


    if(Vector3Distance(gst->player.position, part->position) > RADIUS) {
        cloud_psys_init(gst, psys, part, (Vector3){0}, (Vector3){0}, (Color){0}, NULL, NO_EXTRADATA);
    }
}



// PARTICLE INITIALIZATION
void cloud_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    
    part->position = (Vector3) {
        gst->player.position.x + RSEEDRANDOMF(-RADIUS, RADIUS),
        RSEEDRANDOMF(-100, 600) + 1200,
        gst->player.position.z + RSEEDRANDOMF(-RADIUS, RADIUS)
    };
 
    part->velocity = (Vector3) {
        RSEEDRANDOMF(1.0, 3.0), 0, RSEEDRANDOMF(1.0, 3.0)
    };
    /*
    part->velocity = (Vector3) {
        0, 0, RSEEDRANDOMF(-500, -100)
    };
    */

    part->lifetime = 0.0;
    part->scale = RSEEDRANDOMF(10.0, 50.0);
}


