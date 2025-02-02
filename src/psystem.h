#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <raylib.h>
#include <stddef.h>

struct state_t;


struct particle_t {
    
    Vector3 velocity;
    Color   color;
    
    float   lifetime;
    float   max_lifetime;
    
    int     alive;
    size_t  index;   


    // "read only"  updated by 'update_psystem'
    // change the transformation matrix instead.
    // corresponding matrix is found at 'psystem->transforms[particle->index]'
    Vector3 position; 

};


struct psystem_t {

    int enabled;

    //int one_shot; // if set to 1 particle system will be disabled after all particles are dead.
    
    struct particle_t* particles;
    Matrix*     transforms;
    Material    particle_material;
    Mesh        particle_mesh;
    Shader      particle_shader;

    size_t max_particles;

    // TODO: pointer "mask" for alive particles.

    // called to update each particle.
    void(*update_callback)(struct state_t*, struct psystem_t*, struct particle_t*);
    
    // called by 'add_particles' for each particle after adding them to the array.
    void(*pinit_callback)(struct state_t*, struct psystem_t*, struct particle_t*);

    void* userptr;
};


void delete_psystem(struct psystem_t* psys);
void create_psystem(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*)
        );


void update_psystem(struct state_t* gst, struct psystem_t* psys);

void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n /* particles to be added */
        );





#endif
