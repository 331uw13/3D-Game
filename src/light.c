#include <stdio.h>

#include "state.h"
#include "light.h"


struct light_t* add_light(
        struct state_t* gst,
        int light_type,
        Vector3 position,
        Color color,
        Shader shader
){
    struct light_t* ptr = NULL;
    if((gst->num_lights+1) >= MAX_LIGHTS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Max lights reached.\033[0m\n",
                __func__);
        goto error;
    }

    const size_t index = gst->num_lights;

    ptr = &gst->lights[index];
    ptr->enabled = 1;
    ptr->type = light_type;
    ptr->position = position;
    ptr->color = color;

    ptr->locs[LIGHT_ENABLED_LOC]  = GetShaderLocation(shader, TextFormat("lights[%i].enabled", index));
    ptr->locs[LIGHT_TYPE_LOC]     = GetShaderLocation(shader, TextFormat("lights[%i].type", index));
    ptr->locs[LIGHT_POSITION_LOC] = GetShaderLocation(shader, TextFormat("lights[%i].position", index));
    ptr->locs[LIGHT_COLOR_LOC]    = GetShaderLocation(shader, TextFormat("lights[%i].color", index));

    update_light_values(ptr, shader);
    gst->num_lights++;

error:
    return ptr;
}


void update_light_values(struct light_t* light, Shader shader) {
    SetShaderValue(shader, light->locs[LIGHT_ENABLED_LOC], &light->enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light->locs[LIGHT_TYPE_LOC], &light->type, SHADER_UNIFORM_INT);

    float position[3] = {
        light->position.x,
        light->position.y, 
        light->position.z
    };
    SetShaderValue(shader, light->locs[LIGHT_POSITION_LOC], position, SHADER_UNIFORM_VEC3);
    
    float color[4] = { 
        (float)light->color.r/255.0,
        (float)light->color.g/255.0,
        (float)light->color.b/255.0,
        1.0
    };
    SetShaderValue(shader, light->locs[LIGHT_COLOR_LOC], color, SHADER_UNIFORM_VEC4);
}

