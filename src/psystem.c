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
    psys->num_alive_parts = 0;
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


static void _remove_pdata(struct psystem_t* psys, struct particle_t* p) {

    if(psys->num_alive_parts == 0) {
        return;
    }

    const size_t starti = p->transf_index;
    for(size_t i = starti; i < psys->num_alive_parts; i++) {
        
        psys->transforms[i] = psys->transforms[i+1];//MatrixTranslate(0, 1, 3);

    }


    // correct the trasnform index for particles.
   
    for(size_t i = p->index; i < psys->max_particles; i++) {
       
        struct particle_t* ptr = &psys->particles[i];

        if(!ptr->alive) {
            continue;
        }
        if(ptr->transf_index > 0) {
            ptr->transf_index--;
        }

        ptr->transform = &psys->transforms[ptr->transf_index];
    }


    psys->num_alive_parts--;

}


// push new data for matrix transforms array
//
static Matrix* _push_pdata(struct psystem_t* psys, struct particle_t* p) {
    Matrix* ptr = NULL;

    if(psys->num_alive_parts+1 >= psys->max_particles) {
        printf(" --- MAX PARTICLES REACHED ---\n");
        goto error;
    }

    const size_t index = psys->num_alive_parts;


    p->transform = &psys->transforms[index];
    p->transf_index = index;



    psys->num_alive_parts++;
    ptr = p->transform;

error:
    return ptr;
}


void update_psystem(struct state_t* gst, struct psystem_t* psys) {

    
    for(size_t i = 0; i < psys->max_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        if(!p->alive) {
            continue;
        }

        psys->update_callback(gst, psys, p);
     

        p->lifetime += gst->dt;
        if(p->lifetime > p->max_lifetime) {
            p->alive = 0;

            psys->pinit_callback(gst, psys, p);
            //_remove_pdata(psys, p);

        }
    }
    


    
    DrawMeshInstanced(
            psys->particle_mesh,
            psys->particle_material,
            psys->transforms,
            psys->num_alive_parts
            );


}


static struct particle_t* _get_dead_particle(struct psystem_t* psys) {
    struct particle_t* deadp = NULL;

    deadp = &psys->particles[psys->nextpart_index];
    
    if(deadp->alive) {

        // TODO:
        // first try to search any dead particle
        // if not found select random alive one.

        deadp = &psys->particles[0];

    }

    if(deadp) {
        psys->nextpart_index++;
        if(psys->nextpart_index >= psys->max_particles) {
            psys->nextpart_index = 0;
        }
    }


error:
    return deadp;
}



void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n /* particles to be added */
        )
{


    for(size_t i = 0; i < n; i++) {
        struct particle_t* p = _get_dead_particle(psys);
        if(!p) {
            break;
        }

        if(!(p->transform = _push_pdata(psys, p))) {
            break;
        }


        // callback to initialize the particle.
        psys->pinit_callback(gst, psys, p);

    }


}


