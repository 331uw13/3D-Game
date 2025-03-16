#include <stdio.h>

#include "state.h"
#include "light.h"


void set_light(
        struct state_t* gst,
        struct light_t* light,
        unsigned int ubo
){

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
        = { light->strength, 0.0, 0.0, 0.0 };

    size_t offset;
    size_t size;


    // TYPE
    offset = (light->index * LIGHT_SHADER_STRUCT_SIZE) + 0;
    size = 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->type);

    // SETTINGS
    offset = (light->index * LIGHT_SHADER_STRUCT_SIZE) + 4;
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->enabled);

    // COLOR
    offset = (light->index * LIGHT_SHADER_STRUCT_SIZE) + 16;
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, color4f);

    // POSITION
    offset = (light->index * LIGHT_SHADER_STRUCT_SIZE) + (16*2);
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pos3f);

    // STRENGTH
    offset = (light->index * LIGHT_SHADER_STRUCT_SIZE) + (16*3);
    size = sizeof(float) * 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, s3f);


    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void disable_light(struct state_t* gst, struct light_t* light, unsigned int ubo) {
    if(light->index >= MAX_PROJECTILE_LIGHTS) {
        return;
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    light->enabled = 0;
   
    size_t offset = (light->index * LIGHT_SHADER_STRUCT_SIZE) + 4;
    size_t size = 4;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &light->enabled);


    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

