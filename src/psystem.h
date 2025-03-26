#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <raylib.h>
#include <stddef.h>

#include "light.h"


struct state_t;

#define PSYS_NO_COLOR_VBO 0xFFFF


struct particle_t {
    
    Vector3 velocity;
    Vector3 accel;
    Color   color; // (Not used if 'psystem->color_vbo' is not setup)
    
    float   lifetime;
    float   max_lifetime;
    float   n_lifetime; // Normalized lifetime.

    float   scale;
    float   initial_scale;

    int     alive;
    size_t  index; // index in psystem 'particles'

    // "read only"
    // change the transformation matrix instead.
    // corresponding matrix is found at 'psystem->transforms[particle->index]'
    Vector3 position;
    Vector3 prev_position; // previous position (updated by 'update_psystem').
    Vector3 origin;

    Color start_color;
    Color end_color;

    // pointing to corresponding location for this particle
    // in psystem_t 'transforms'
    Matrix* transform;

    void* extradata;
    
    struct light_t light;
    int has_light;
    int last_update;

    int user_i[3];
    Vector3 user_v[3];

    int idb; // Behaviour ID.
};

#define NO_EXTRADATA 0
#define HAS_EXTRADATA 1

#define PSYS_GROUPID_PLAYER 0
#define PSYS_GROUPID_ENEMY 1
#define PSYS_GROUPID_ENV 2

// 'time_setting' for particle system
#define PSYS_ONESHOT 0
#define PSYS_CONTINUOUS 1

#define NO_IDB -1


struct psystem_t {

    int groupid;
    int enabled;
    struct particle_t* particles;
    
    Matrix*     transforms;
    
    Material    particle_material;
    Mesh        particle_mesh;

    size_t max_particles;
    size_t num_alive_parts;

    size_t nextpart_index;
    

    // Called to update each particle.
    void(*update_callback)(
            struct state_t*,
            struct psystem_t*,
            struct particle_t* // Current particle
            );
    
    // Called by 'add_particles' for each particle after adding them to the array.
    void(*pinit_callback)(
         struct state_t*,
         struct psystem_t*,
         struct particle_t*, // Current particle
         Vector3,  // Particle initial position
         Vector3, // Velocity to new particle
         void*,   // Extra data pointer
         int      // Has extra data?
         );

    int shader_index; // Index in 'gst->shaders' array.
    
    // uniformlocs are set to negative value if not used.
    int shader_color_uniformloc;
    int shader_time_uniformloc;

    int time_setting;

    int   first_render;
    int   halt;
    void* userptr;

    size_t color_vbo_size;
    unsigned int color_vbo;

};


void delete_psystem(struct psystem_t* psys);


// IMPORTANT NOTE: do NOT set psystem.userptr before calling 'create_psystem'. set it AFTER.
void create_psystem(
        struct state_t* gst,
        int groupid,
        int time_setting,
        struct psystem_t* psys,
        size_t max_particles,
        void(*update_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*),
        void(*pinit_callback_ptr)(struct state_t*, struct psystem_t*, struct particle_t*, Vector3,Vector3,void*,int),
        int shader_index // See state.h
        );

// Setup vbo to handle colors for each particle.
// IMPORTANT NOTE: if using individual colors for each mesh drawn with 'DrawMeshInstanced'
//                 this function must be called after particle system has been created!
void setup_psystem_color_vbo(struct state_t* gst, struct psystem_t* psys);

void psystem_set_idb(struct psystem_t* psys, int id, size_t num);

void update_psystem(struct state_t* gst, struct psystem_t* psys);
void render_psystem(struct state_t* gst, struct psystem_t* psys, Color color);

void add_particles(
        struct state_t* gst,
        struct psystem_t* psys,
        size_t n, /* Particles to be added */
        Vector3 origin,
        Vector3 velocity,
        void* extradata_ptr,
        int has_extradata,
        int idb
        );

void disable_particle(struct state_t* gst, struct particle_t* p);




#endif
