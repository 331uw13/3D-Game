#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <raylib.h>
#include <stddef.h>

#include "light.h"


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

    struct light_t light;
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
    void(*update_callback)(
            struct state_t*,
            struct psystem_t*,
            struct particle_t* // current particle
            );
    
    // called by 'add_particles' for each particle after adding them to the array.
    void(*pinit_callback)(
         struct state_t*,
         struct psystem_t*,
         struct particle_t*, // current particle
         Vector3,  // particle initial position
         Vector3, // velocity to new particle
         void*,   // extra data pointer
         int      // has extra data?
         );

    void* userptr;
};


void delete_psystem(struct psystem_t* psys);

// IMPORTANT NOTE: do NOT set psystem.userptr before calling 'create_psystem'. set it AFTER.
void create_psystem(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, Vector3, Vector3, void*, int)
        );


void update_psystem(struct state_t* gst, struct psystem_t* psys);

void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n, /* particles to be added */
        Vector3 origin,
        Vector3 velocity,
        void* extradata_ptr,
        int has_extradata
        );





#endif
