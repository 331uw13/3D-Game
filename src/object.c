#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "util.h"

#include "state.h"

/*
void free_objarray(struct state_t* gst) {
    if(gst->objects) {   
        for(size_t i = 0; i < gst->num_objects; i++) {
            
            if(gst->objects[i].model_loaded) {
                UnloadModel(gst->objects[i].model);
                gst->objects[i].model_loaded = 0;
            }
        }

        free(gst->objects);
    }
    
    printf("\033[32m >> Deleted Objects.\033[0m\n");
}


// returns the id if succesful if not -1 is returned.
struct obj_t* create_object(
        struct state_t* gst,
        const char* model_filepath,
        int texture_id,
        Vector3 init_pos
        )
{
    struct obj_t* newobjptr = NULL;

    long int new_num_elem = 0;
    gst->objects = m_resize_array(
            gst->objects,
            sizeof *gst->objects,
            gst->objarray_size,
            gst->objarray_size + 1,
            &new_num_elem
            );
    if(new_num_elem == MEMRESIZE_ERROR) {
        goto error;
    }


    gst->objarray_size = new_num_elem;

    long int id = gst->num_objects;
    gst->num_objects++;

    newobjptr = &gst->objects[id];
    newobjptr->id = id;
    newobjptr->model_loaded = setup_3Dmodel(gst, &newobjptr->model, model_filepath, texture_id, init_pos);
    if(!newobjptr->model_loaded) {
        goto error;
    }
    
    
    printf("\033[34m >> Created Object. ID(%li), Model '%s'\033[35m objarray_size: %li\033[0m\n", 
            id, model_filepath, gst->objarray_size);

error:
    return newobjptr;
}
*/




