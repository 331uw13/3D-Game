#include <stdio.h>
#include <stdlib.h>

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

    psys->enabled = 0;
    psys->update_callback = NULL;
    psys->pinit_callback = NULL;
}

void create_psystem(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*)
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

        int n = 100;
        float x = GetRandomValue(-n, n);
        float y = GetRandomValue(-n, n);
        float z = GetRandomValue(-n, n);

        p->velocity = (Vector3){ 
            GetRandomValue(-3.0, 3.0) / 3.0,
            GetRandomValue(-3.0, 3.0) / 3.0,
            GetRandomValue(-3.0, 3.0) / 3.0
        };


        psys->transforms[i] = MatrixTranslate(x,y,z);
    
        psys->pinit_callback(gst, psys, p);
    }


    psys->enabled = 1;
}


void update_psystem(struct state_t* gst, struct psystem_t* psys) {

    for(size_t i = 0; i < psys->max_particles; i++) {
        struct particle_t* p = &psys->particle[i];

        psys->update_callback(gst, psys, p);
     

        p->lifetime += gst->dt;
        if(p->lifetime > p->max_lifetime) {
            
        }



    }


    DrawMeshInstanced(
            psys->particle_mesh,
            psys->particle_material,
            psys->transforms,
            psys->max_particles
            );

}


void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n /* particles to be added */
        )
{


}


