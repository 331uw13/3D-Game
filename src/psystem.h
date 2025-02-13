#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <raylib.h>
#include <stddef.h>

struct state_t;


struct particle_t {
    
    Vector3 velocity;
    Vector3 accel;
    Color   color;
    
    float   lifetime;
    float   max_lifetime;

    float   scale;
    float   initial_scale;

    int     alive;
    size_t  index; // index in psystem 'particles'

    // "read only"
    // change the transformation matrix instead.
    // corresponding matrix is found at 'psystem->transforms[particle->index]'
    Vector3 position;
    Vector3 prev_position; // previous position (updated by 'update_psystem').

    // pointing to corresponding location for this particle
    // in psystem_t 'transforms'
    Matrix* transform;

    void* extradata;

};

#define NO_EXTRADATA 0
#define HAS_EXTRADATA 1


struct psystem_t {

    int enabled;

    // if "true": particle system will be disabled after all particles are dead.
    //            and 'pinit_callback' is not called after particle dies.
    int one_shot; // <- TODO
    
    struct particle_t* particles;
    
    Matrix*     transforms;
    
    Material    particle_material;
    Mesh        particle_mesh;
    Shader      particle_shader;

    size_t max_particles;
    size_t num_alive_parts;

    size_t nextpart_index;

    // called to update each particle.
    void(*update_callback)(struct state_t*, struct psystem_t*, struct particle_t*);
    
    // called by 'add_particles' for each particle after adding them to the array.
    void(*pinit_callback)(struct state_t*, struct psystem_t*, struct particle_t*, void*, int);

    void* userptr;
};


void delete_psystem(struct psystem_t* psys);
void create_psystem(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, void*, int)
        );


void update_psystem(struct state_t* gst, struct psystem_t* psys);

void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n, /* particles to be added */
        void* extradata_ptr,
        int has_extradata
        );





#endif
