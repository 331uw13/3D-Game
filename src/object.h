#ifndef OBJECT_H
#define OBJECT_H

#include <raylib.h>
#include <stddef.h>



// handles static objects.
// TODO: user can interact with objects?


struct state_t;


struct obj_t {
    long int id;
    int model_loaded; // model may be unloaded to save memory.
    Model model;
};


void  free_objarray(struct state_t* gst); // also unloads all the object's models
int   setup_obj_model(struct state_t* gst, struct obj_t* obj, const char* model_filepath,
        float init_x, float init_y, float init_z);


// returns NULL if not succesful.
struct obj_t* create_object(
        struct state_t* gst,
        const char* model_filepath,
        int texture_id,
        Vector3 init_pos
        );

/*  TODO
// NOTE: this function does not unload the object's model!
int remove_obj(struct state_t* gst, long int id);

// 'delete_obj' may create fragmentation so this function is good to call sometimes.
// but not always because it may get expensive if there are alot of objects.
void reorder_objects(struct state_t* gst);
*/


#endif
