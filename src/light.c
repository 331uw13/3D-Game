#include <stdio.h>

#include "state.h"
#include "light.h"


void add_light(
        struct state_t* gst,
        int light_type,
        Vector3 position,
        Color color,
        Shader shader
){

    if((gst->num_normal_lights+1) >= MAX_NORMAL_LIGHTS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Trying to create too many \"normal lights\"\033[0m\n",
                __func__);
        return;
    }


    const size_t index = gst->num_normal_lights;
    struct light_t* light = &gst->normal_lights[index];


    light->enabled = 1;
    light->type = light_type;
    light->position = position;
    light->color = color;

    light->locs[LIGHT_ENABLED_LOC]  = GetShaderLocation(shader, TextFormat("lights[%i].enabled",  index));
    light->locs[LIGHT_TYPE_LOC]     = GetShaderLocation(shader, TextFormat("lights[%i].type",     index));
    light->locs[LIGHT_POSITION_LOC] = GetShaderLocation(shader, TextFormat("lights[%i].position", index));
    light->locs[LIGHT_COLOR_LOC]    = GetShaderLocation(shader, TextFormat("lights[%i].color",    index));

   
    update_light_values(light, shader);
    gst->num_normal_lights++;
}





void add_projectile_light(
        struct state_t* gst,
        struct light_t* light,
        Vector3 position,
        Color color,
        Shader shader
){

    const size_t index = gst->num_projectile_lights;

    light->enabled = 1;
    light->type = LIGHT_POINT;
    light->position = position;
    light->color = color;

    light->locs[LIGHT_ENABLED_LOC]  = GetShaderLocation(shader, TextFormat("prj_lights[%i].enabled",  index));
    light->locs[LIGHT_TYPE_LOC]     = GetShaderLocation(shader, TextFormat("prj_lights[%i].type",     index));
    light->locs[LIGHT_POSITION_LOC] = GetShaderLocation(shader, TextFormat("prj_lights[%i].position", index));
    light->locs[LIGHT_COLOR_LOC]    = GetShaderLocation(shader, TextFormat("prj_lights[%i].color",    index));

   
    update_light_values(light, shader);
  
    gst->num_projectile_lights++;
    if(gst->num_projectile_lights >= MAX_PROJECTILE_LIGHTS) {
        gst->num_projectile_lights = 0;
    }
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

void disable_light(struct light_t* light, Shader shader) {
    light->enabled = 0;
    SetShaderValue(shader, light->locs[LIGHT_ENABLED_LOC], &light->enabled, SHADER_UNIFORM_INT);
}
