#include <stdio.h>

#include "state/state.h"
#include "light.h"


static uint16_t g_unique_id_counter = 0;


int add_light(struct state_t* gst, struct chunk_t* chunk, struct light_t* light) {
    int result = 0;

    int new_index = MAX_LIGHTS_PERCHUNK-1;

    for(int i = 0; i < MAX_LIGHTS_PERCHUNK; i++) {
        if(!chunk->lights[i]) {
            new_index = i;
            break;
        }

        if(!chunk->lights[i]->enabled) {
            new_index = i;
            break;
        }
    }


    light->enabled = 1;
    light->chunk = chunk;
    chunk->lights[new_index] = light;

    result = 1;

error:
    return result;
}

void remove_light(struct state_t* gst, struct light_t* light) {
    light->enabled = 0;
}



