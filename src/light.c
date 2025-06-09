#include <stdio.h>

#include "state/state.h"
#include "light.h"





struct light_t* add_light(struct state_t* gst, struct chunk_t* chunk, struct light_t light_settings) {
    struct light_t* light = NULL;

    if(chunk->num_lights+1 >= MAX_LIGHTS_PERCHUNK) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Too many lights in chunk %li\033[0m\n",
                __func__, chunk->index);
        goto error;
    }

    if(!chunk) {
        chunk = find_chunk(gst, (Vector3){ 0, 0, 0 });
        fprintf(stderr, "\033[33m(WARNING) '%s': Chunk pointer should not be NULL.\033[0m\n"
                        " It has beens set to %p to avoid crashing.\n",
                __func__, chunk);
    }

    
    // Print warnings for lights settings if needed.
    
    if((light_settings.strength > 100000.0) || (light_settings.strength < -100000.0)) {
        fprintf(stderr, 
                "\033[33m(WARNING) '%s': Light's strength is very big or small number."
                " Is it set correctly?\033[0m\n",
                __func__);
    }
    if((light_settings.radius > 100000.0) || (light_settings.radius < -100000.0)) {
        fprintf(stderr, 
                "\033[33m(WARNING) '%s': Light's radius is very big or small number."
                " Is it set correctly?\033[0m\n",
                __func__);
    }

    // Copy light settings to chunk's light array.
    light = &chunk->lights[chunk->num_lights];
    *light = light_settings;
   
    light->chunk = chunk;
    light->index = chunk->num_lights;

    chunk->num_lights++;



    printf("Added light to chunk %li (%p) \n",
            chunk->index, chunk);

error:
    return light;
}


void remove_light(struct state_t* gst, struct light_t* light) {
    struct chunk_t* chunk = light->chunk;
    if(chunk->num_lights <= 0) {
        return;
    }

    for(uint16_t i = light->index; i < chunk->num_lights-1; i++) {        
        chunk->lights[i] = chunk->lights[i+1];
        if(chunk->lights[i].index > 0) {
            chunk->lights[i].index--;
        }
    }

    chunk->num_lights--;
}


