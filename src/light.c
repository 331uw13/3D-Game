#include <stdio.h>

#include "state/state.h"
#include "light.h"



struct light_t* add_light(struct state_t* gst, struct light_t light_settings, int can_overwrite) {

    struct light_t* ptr = &gst->lights[gst->next_disabled_light];
    if(ptr->enabled) {
        // Low possibility to happen but now disabled light index must be found.
        int found = 0;
        for(uint16_t i = 0; i < MAX_LIGHTS; i++) {
            struct light_t* light = &gst->lights[i];
            if(!light->enabled) {
                found = 1;
                ptr = light;
                break;
            }
        }

        if(!found) {
            if(!can_overwrite) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Light array is full.\033[0m\n",
                        __func__);
                return NULL;
            }

            uint16_t new_index = GetRandomValue(0, MAX_LIGHTS-1);
            ptr = &gst->lights[new_index];
        }
    }


    *ptr = light_settings;
    ptr->enabled = 1;

    return ptr;
}

void remove_light(struct state_t* gst, struct light_t* light) { 
    gst->next_disabled_light = light->index;
    light->enabled = 0;
}



