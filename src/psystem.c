#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psystem.h"
#include <raymath.h>
#include "state.h"




void delete_psystem(struct psystem_t* psys) {

    if(psys->particles) {
        free(psys->particles);
        psys->particles = NULL;
    }

    if(psys->transforms) {
        free(psys->transforms);
        psys->transforms = NULL;
    }

    UnloadMesh(psys->particle_mesh);

    psys->enabled = 0;
    psys->update_callback = NULL;
    psys->pinit_callback = NULL;
}

void create_psystem(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, void*, int)
        )
{

    if(!update_callback_ptr) {
        fprintf(stderr, " >> (ERROR) '%s' Update callback pointer is missing for particle system.\n",
                __func__);
        return;
    }

    if(!update_callback_ptr) {
        fprintf(stderr, " >> (ERROR) '%s' Particle initialization callback pointer is missing for particle system.\n",
                __func__);
        return;
    }

    psys->enabled = 0;
    psys->max_particles = max_particles;
    psys->num_alive_parts = 0;
    psys->nextpart_index = 0;
    psys->particles = NULL;
    psys->transforms = NULL;
    psys->update_callback = update_callback_ptr;
    psys->pinit_callback = pinit_callback_ptr;

    psys->particles = calloc(max_particles, sizeof *psys->particles);
    if(!psys->particles) {
        fprintf(stderr, " >> (ERROR) '%s' Failed to allocate memory for particles\n",
                __func__);
        perror("calloc");
        return;
    }

    psys->transforms = calloc(max_particles, sizeof *psys->transforms);
    if(!psys->transforms) {
        fprintf(stderr, " >> (ERROR) '%s' Failed to allocate memory for particle transformation matrices\n",
                __func__);
        perror("calloc");
        return;
    }


    for(size_t i = 0; i < max_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        p->index = i;
    }


    psys->enabled = 1;
}


static struct particle_t* _add_particle(struct psystem_t* psys) {

    struct particle_t* p = &psys->particles[psys->nextpart_index];
    p->transform = &psys->transforms[psys->nextpart_index];

    psys->nextpart_index++;
    if(psys->nextpart_index >= psys->max_particles) {
        psys->nextpart_index = 0;
    }

    return p;
}


void update_psystem(struct state_t* gst, struct psystem_t* psys) {

    psys->num_alive_parts = 0;
    
    for(size_t i = 0; i < psys->max_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        if(!p->alive) {
            continue;
        }

        psys->num_alive_parts++;

        psys->update_callback(gst, psys, p);
        p->prev_position = p->position;

        p->lifetime += gst->dt;
        if(p->lifetime > p->max_lifetime) {
            p->alive = 0;

            //psys->pinit_callback(gst, psys, p);

        }
    }


    DrawMeshInstanced(
            psys->particle_mesh,
            psys->particle_material,
            psys->transforms,
            psys->max_particles
            );


    // clear the transform matrix array for next frame.
    memset(psys->transforms, 0, psys->max_particles * sizeof *psys->transforms);
}





void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n, /* particles to be added */
        void* extradata_ptr,
        int has_extradata
        )
{


    for(size_t i = 0; i < n; i++) {

        struct particle_t* p = _add_particle(psys);

        // callback to initialize the particle.
        psys->pinit_callback(gst, psys, p, extradata_ptr, (has_extradata && extradata_ptr));

    }


}


