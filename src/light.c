#include <stdio.h>

#include "state.h"
#include "light.h"


void set_light(
        struct state_t* gst,
        struct light_t* light,
        int ubo_index
){
    unsigned int ubo = gst->ubo[ubo_index];

    if(light->index >= MAX_PROJECTILE_LIGHTS) {
        return;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    float color4f[4] = { 
        (float)light->color.r/255.0,
        (float)light->color.g/255.0,
        (float)light->color.b/255.0,
        (float)light->color.a/255.0
    };

    float pos3f[4]
        = { light->position.x, light->position.y, light->position.z, 0.0 };

    float s3f[4] // Maybe will add more settings later..
        = { light->strength, light->radius, 0.0, 0.0 };

    size_t offset;
    size_t size;


    // TYPE
    offset = (light->index * LIGHT_UB_STRUCT_SIZE) + 0;
    size = 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->type);

    // SETTINGS
    offset = (light->index * LIGHT_UB_STRUCT_SIZE) + 4;
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->enabled);

    // COLOR
    offset = (light->index * LIGHT_UB_STRUCT_SIZE) + 16;
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, color4f);

    // POSITION
    offset = (light->index * LIGHT_UB_STRUCT_SIZE) + (16*2);
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pos3f);

    // STRENGTH
    offset = (light->index * LIGHT_UB_STRUCT_SIZE) + (16*3);
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, s3f);


    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    light->ubo_index = ubo_index;
}

void disable_light(struct state_t* gst, struct light_t* light, int ubo_index) {

    unsigned int ubo = gst->ubo[ubo_index];

    if(light->index >= MAX_PROJECTILE_LIGHTS) {
        return;
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    light->enabled = 0;
   
    size_t offset = (light->index * LIGHT_UB_STRUCT_SIZE) + 4;
    size_t size = 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->enabled);


    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void add_decay_light(struct state_t* gst, struct light_t* light, float decay_time_mult) {
    struct light_t* dlight = NULL;

    for(size_t i = 0; i < MAX_DECAY_LIGHTS; i++) {
        if(gst->decay_lights[i].enabled) {
            continue;
        }

        dlight = &gst->decay_lights[i];
    }

    if(!dlight) {
        disable_light(gst, light, light->ubo_index);
        fprintf(stderr, "\033[35m(WARNING) '%s': Too many lights decaying.\033[0m\n",
                __func__);
        return;
    }


    *dlight = *light;
    dlight->decay = decay_time_mult;

}

void update_decay_lights(struct state_t* gst) {
    for(size_t i = 0; i < MAX_DECAY_LIGHTS; i++) {
        struct light_t* light = &gst->decay_lights[i];

        light->radius -= gst->dt * light->decay;

        if(light->radius > 0.0) {
            set_light(gst, light, light->ubo_index);
        }
        else {
            disable_light(gst, light, light->ubo_index);
        }
    }
}


