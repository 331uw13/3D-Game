#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psystem.h"

#include "state.h"
#include "util.h"



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
    
    printf(" >> Deleted ParticleSystem\n");
}

void create_psystem(
        struct state_t* gst,
        int groupid,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, Vector3,Vector3,void*,int),
        int shader_index
){
    psys->halt = 1;

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

    psys->groupid = groupid;
    psys->enabled = 0;
    psys->max_particles = max_particles;
    psys->num_alive_parts = 0;
    psys->nextpart_index = 0;
    psys->particles = NULL;
    psys->transforms = NULL;
    psys->update_callback = update_callback_ptr;
    psys->pinit_callback = pinit_callback_ptr;
    psys->userptr = NULL;


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

    psys->shader_index = shader_index;

    psys->shader_color_uniformloc = GetShaderLocation(gst->shaders[shader_index], "psystem_color");
    psys->shader_time_uniformloc = GetShaderLocation(gst->shaders[shader_index], "time");

    psys->particle_material = LoadMaterialDefault();
    psys->particle_material.shader = gst->shaders[shader_index];


    psys->halt = 0;
    psys->first_render = 1;
    psys->enabled = 1;
}


static struct particle_t* _add_particle(struct psystem_t* psys) {
    struct particle_t* p = &psys->particles[psys->nextpart_index];
    p->transform = &psys->transforms[psys->nextpart_index];

    p->alive = 1;
    p->lifetime = 0.0;
    p->has_light = 0;

    psys->nextpart_index++;
    if(psys->nextpart_index >= psys->max_particles) {
        psys->nextpart_index = 0;
    }

    return p;
}


void update_psystem(struct state_t* gst, struct psystem_t* psys) {

    psys->num_alive_parts = 0;
   
    if(!psys->update_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s': No update callback.\033[0m\n",
                __func__);
        return;
    }

    int num_rendering = 0;

    for(size_t i = 0; i < psys->max_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        if(!p) {
            continue;
        }
        if(!p->alive) {
            continue;
        }

        if(p->max_lifetime <= 0.0) {
            fprintf(stderr, "\033[35m(WARNING) '%s': Particle 'max_lifetime' is 0 or less\033[0m\n",
                    __func__);
        }

        psys->num_alive_parts++;
        psys->update_callback(gst, psys, p);

        p->prev_position = p->position;


        p->lifetime += gst->dt;
        if(p->lifetime > p->max_lifetime) {
            p->alive = 0;

            if(p->has_light) {
                disable_light(gst, &p->light, gst->prj_lights_ubo);
            }

            continue;
            // TODO -> psys->pinit_callback(gst, psys, p);
        }


    }

}

void render_psystem(struct state_t* gst, struct psystem_t* psys, Color color) {
    if(psys->halt) {
        return;
    }

    // Safety check first render.
    // TODO. add 'tag' in psystem to see where these errors may be coming from?
    if(psys->first_render) {
        if(!IsMaterialValid(psys->particle_material)) {
            fprintf(stderr, "\033[31m\033[7m(ERROR) '%s': 'particle_material' is not loaded.\033[0m\n",
                    __func__);
            psys->halt = 1;
            return;
        }
        if(!IsShaderValid(psys->particle_material.shader)) {
            fprintf(stderr, "\033[31m\033[7m(ERROR) '%s': 'particle_shader' is not loaded.\033[0m\n",
                    __func__);
            psys->halt = 1;
            return;
        }
        if(!(psys->particle_mesh.vertices)) {
            fprintf(stderr, "\033[31m\033[7m(ERROR) '%s': 'particle_mesh' is not loaded.\033[0m\n",
                    __func__);
            psys->halt = 1;
            return;
        }

        psys->first_render = 0;
    }


    // TODO: optimize these uniform things.

    const float psystem_color[4] = {
        (float)color.r / 255.0,
        (float)color.g / 255.0,
        (float)color.b / 255.0,
        (float)color.a / 255.0,
    };

    SetShaderValue(gst->shaders[psys->shader_index], psys->shader_color_uniformloc,
            psystem_color, SHADER_UNIFORM_VEC4);


    SetShaderValue(gst->shaders[psys->shader_index], psys->shader_time_uniformloc,
            &gst->time, SHADER_UNIFORM_FLOAT);
    
    
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
        Vector3 origin,
        Vector3 velocity,
        void* extradata_ptr,
        int has_extradata
        )
{
    for(size_t i = 0; i < n; i++) {
        struct particle_t* p = _add_particle(psys);

        // callback to initialize the particle.
        psys->pinit_callback(gst, psys, p, origin, velocity, extradata_ptr, (has_extradata && extradata_ptr));
    }
}

void disable_particle(struct state_t* gst, struct particle_t* p) {
    p->alive = 0;
    p->lifetime = p->max_lifetime;
    if(p->has_light) {
        disable_light(gst, &p->light, gst->prj_lights_ubo);
        p->has_light = 0;
    }
}

