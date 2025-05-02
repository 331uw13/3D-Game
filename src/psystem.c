#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psystem.h"

#include "state/state.h"
#include "util.h"

#include <rlgl.h>


void delete_psystem(struct psystem_t* psys) {

    if(psys->particles) {
        free(psys->particles);
        psys->particles = NULL;
    }

    if(psys->transforms) {
        free(psys->transforms);
        psys->transforms = NULL;
    }

    if(psys->color_vbo != PSYS_NO_COLOR_VBO) {
        glDeleteBuffers(1, &psys->color_vbo);
    }

    if(IsModelValid(psys->particle_model)) {
        UnloadModel(psys->particle_model);
    }

    psys->enabled = 0;
    psys->update_callback = NULL;
    psys->pinit_callback = NULL;
}

void create_psystem(
        struct state_t* gst,
        int groupid,
        int time_setting,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, 
            Vector3,Vector3,Color,void*,int),
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
    psys->time_setting = time_setting;
    psys->enabled = 0;
    psys->max_particles = max_particles;
    psys->num_alive_parts = 0;
    psys->nextpart_index = 0;
    psys->particles = NULL;
    psys->transforms = NULL;
    psys->update_callback = update_callback_ptr;
    psys->pinit_callback = pinit_callback_ptr;
    psys->userptr = NULL;
    psys->particle_model_mesh_index = 0;

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

    psys->color_vbo = PSYS_NO_COLOR_VBO;

    psys->halt = 0;
    psys->first_render = 1;
    psys->enabled = 1;


    for(size_t i = 0; i < PSYS_MAX_USER_I; i++) {
        psys->user_i[i] = 0;
    }
    for(size_t i = 0; i < PSYS_MAX_USER_F; i++) {
        psys->user_f[i] = 0;
    }
    for(size_t i = 0; i < PSYS_MAX_USER_V; i++) {
        psys->user_v[i] = (Vector3){0};
    }
    for(size_t i = 0; i < PSYS_MAX_USER_P; i++) {
        psys->user_p[i] = NULL;
    }
}

void setup_psystem_color_vbo(struct state_t* gst, struct psystem_t* psys) {
    if(psys->color_vbo != PSYS_NO_COLOR_VBO) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Particle system's color vbo seems to be already created\033[0m\n",
                __func__);
        return;
    }
    if(!psys->enabled || (psys->max_particles == 0)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Particle system must be created before enabling color vbo\033[0m\n",
                __func__);
        return;
    }


    rlEnableVertexArray(psys->particle_model.meshes[0].vaoId);

    psys->color_vbo = (sizeof(float) * 4) * psys->max_particles;
    psys->color_vbo = rlLoadVertexBuffer(NULL, psys->color_vbo, 1);


    int attrib_loc = glGetAttribLocation(gst->shaders[psys->shader_index].id, "instanceColor");
    size_t stride = sizeof(float)*4;

    rlEnableVertexAttribute(attrib_loc);
    rlSetVertexAttribute(attrib_loc, 4, RL_FLOAT, 0, stride, 0);
    rlSetVertexAttributeDivisor(attrib_loc, 1);

    rlDisableVertexArray();
}


void update_psystem(struct state_t* gst, struct psystem_t* psys) {
    if(psys->halt) {
        return;
    }
    if(psys->num_alive_parts == 0) {  
        return;
    }
    if(!psys->update_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s': No update callback.\033[0m\n",
                __func__);
        return;
    }
    if(!psys->pinit_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s': No particle initialization callback.\033[0m\n",
                __func__);
        return;
    }

    int has_color_vbo = (psys->color_vbo != PSYS_NO_COLOR_VBO);

    if(has_color_vbo) {
        rlEnableVertexArray(psys->particle_model.meshes[0].vaoId);
        glBindBuffer(GL_ARRAY_BUFFER, psys->color_vbo);
    }
    
    psys->num_alive_parts = 0;
    for(size_t i = 0; i < psys->max_particles; i++) {
        struct particle_t* p = &psys->particles[i];
        if(!p) {
            continue;
        }
        if(!p->alive) {
            continue;
        }

        if((p->max_lifetime <= 0.0) && (psys->time_setting != PSYS_CONTINUOUS)) {
            fprintf(stderr, "\033[35m(WARNING) '%s': Particle 'max_lifetime' is 0 or less\033[0m\n",
                    __func__);
        }

        if((p->lifetime+gst->dt) >= p->max_lifetime) {
            p->last_update = 1;
        }

        p->n_lifetime = normalize(p->lifetime, 0, p->max_lifetime);
        
        psys->num_alive_parts++;
        psys->update_callback(gst, psys, p);

        p->prev_position = p->position;

        p->lifetime += gst->dt;
        if((psys->time_setting == PSYS_ONESHOT) && (p->lifetime > p->max_lifetime)) {
            p->alive = 0;
            if(p->has_light) {
                disable_light(gst, &p->light, PRJLIGHTS_UBO);
            }

            continue;
        }

        // Update individual colors if needed.

        if(psys->color_vbo != PSYS_NO_COLOR_VBO) {
            const size_t color_size = sizeof(float)*4;
            const size_t offset = p->index * color_size;

            float color_data[4] = {
                (float)p->color.r/255.0,
                (float)p->color.g/255.0,
                (float)p->color.b/255.0,
                (float)p->color.a/255.0
            };

            glBufferSubData(GL_ARRAY_BUFFER, offset, color_size, color_data);

        }
    }
   
    if(has_color_vbo) {
        rlDisableVertexArray();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void render_psystem(struct state_t* gst, struct psystem_t* psys, Color global_psys_color) {
    if(psys->halt) {
        return;
    }

    // Safety check first render.
    // TODO. add 'tag' in psystem to see where these errors may be coming from?
    if(psys->first_render) {
        if(!IsMaterialValid(psys->particle_material)) {
            fprintf(stderr, "\033[31m\033[7m(ERROR) '%s': 'particle_material' is not loaded correctly.\033[0m\n",
                    __func__);
            psys->halt = 1;
            return;
        }
        if(!IsShaderValid(psys->particle_material.shader)) {
            fprintf(stderr, "\033[31m\033[7m(ERROR) '%s': 'particle_shader' is not loaded correctly.\033[0m\n",
                    __func__);
            psys->halt = 1;
            return;
        }
        if(!IsModelValid(psys->particle_model)) {
            fprintf(stderr, "\033[31m\033[7m(ERROR) '%s': 'particle_model' is not loaded correctly.\033[0m\n",
                    __func__);
            psys->halt = 1;
            return;
        }

        psys->first_render = 0;
    }

    if(psys->num_alive_parts == 0) {
        return;
    }



    const float psystem_color[4] = {
        (float)global_psys_color.r / 255.0,
        (float)global_psys_color.g / 255.0,
        (float)global_psys_color.b / 255.0,
        (float)global_psys_color.a / 255.0,
    };

    SetShaderValue(gst->shaders[psys->shader_index], psys->shader_color_uniformloc,
            psystem_color, SHADER_UNIFORM_VEC4);


    SetShaderValue(gst->shaders[psys->shader_index], psys->shader_time_uniformloc,
            &gst->time, SHADER_UNIFORM_FLOAT);



    DrawMeshInstanced(
            psys->particle_model.meshes[psys->particle_model_mesh_index],
            psys->particle_material,
            psys->transforms,
            psys->max_particles
            );
    

    // Clear the transform matrix array for next frame.
    memset(psys->transforms, 0, psys->max_particles * sizeof *psys->transforms);
}


static struct particle_t* _add_particle(struct psystem_t* psys) {
    struct particle_t* p = &psys->particles[psys->nextpart_index];
    p->transform = &psys->transforms[psys->nextpart_index];

    p->forcevec_index = 0;
    p->position = (Vector3){ 0, 0, 0 };
    p->prev_position = (Vector3){ 0, 0, 0 };
    p->origin = (Vector3){ 0, 0, 0 };
    p->velocity = (Vector3){ 0, 0, 0 };
    p->accel = (Vector3){ 0, 0, 0 };
    p->color = (Color){ 0, 0, 0, 255 };
    p->start_color = (Color){ 0, 0, 0, 255 };
    p->end_color = (Color){ 0, 0, 0, 255 };
    p->extradata = NULL;
    p->lifetime = 0.0;
    p->max_lifetime = 0.0;
    p->n_lifetime = 0.0;
    p->scale = 0.0;
    p->initial_scale = 0.0;
    p->alive = 1;
    p->has_light = 0;
    p->last_update = 0;
    p->idb = NO_IDB;

    psys->nextpart_index++;
    if(psys->nextpart_index >= psys->max_particles) {
        psys->nextpart_index = 0;
    }

    return p;
}

struct particle_t* add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n, /* particles to be added */
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata_ptr,
        int has_extradata,
        int idb
        )
{
    struct particle_t* first_part = NULL;

    for(size_t i = 0; i < n; i++) {
        struct particle_t* p = _add_particle(psys);

        p->origin = origin;
        p->idb = idb;

        // Callback to initialize the particle.
        psys->pinit_callback(
                gst,
                psys,
                p,
                origin,
                velocity,
                part_color,
                extradata_ptr,
                (has_extradata && extradata_ptr)
                );
        if(n == 1) {
            first_part = p;
        }
    }

    psys->num_alive_parts = n;
    return first_part;
}

void disable_particle(struct state_t* gst, struct particle_t* p) {
    p->alive = 0;
    p->lifetime = p->max_lifetime;
    
    /*
    if(p->has_light) {
        disable_light(gst, &p->light, PRJLIGHTS_UBO);
        p->has_light = 0;
    }
    */
}


