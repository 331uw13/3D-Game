#include "lib/rlights.h"


#include "projectile_lights.h"
#include "state.h"

#include <stdio.h>
void enable_projectile_light(struct state_t* gst, unsigned int index, Vector3 initpos) {
    
    Light light = { 0 };

    light.enabled = true;
    light.type = LIGHT_POINT;
    light.position = initpos;
    light.target = (Vector3){0.0,0.0,0.0};
    light.color = (Color){ 0, 230, 255, 255};

    int i = gst->next_projlight_index;

    light.enabledLoc = GetShaderLocation(gst->light_shader, TextFormat("projlights[%i].enabled", i));
    light.typeLoc = GetShaderLocation(gst->light_shader, TextFormat("projlights[%i].type", i));
    light.positionLoc = GetShaderLocation(gst->light_shader, TextFormat("projlights[%i].position", i));
    light.targetLoc = GetShaderLocation(gst->light_shader, TextFormat("projlights[%i].target", i));
    light.colorLoc = GetShaderLocation(gst->light_shader, TextFormat("projlights[%i].color", i));

    gst->projectile_lights[index] = light;
    UpdateLightValues(gst->light_shader, light);

    printf("  -> next_projlight_index: %i\n", i);

    gst->next_projlight_index++;
    if(gst->next_projlight_index >= MAX_PROJECTILE_LIGHTS) {
        gst->next_projlight_index = 0;
    }

}

void disable_projectile_light(struct state_t* gst, unsigned int index) {
    
        Light* light = &gst->projectile_lights[index];

        light->enabled = 0;

        printf("DISABLE %i\n", index);
        SetShaderValue(gst->light_shader, light->enabledLoc, &light->enabled, SHADER_UNIFORM_INT);
        
       gst->next_projlight_index = index;
}


