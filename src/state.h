#ifndef GSTATE_H
#define GSTATE_H


#include <rcamera.h> // raylib camera

#define RLIGHTS_IMPLEMENTATION
#include "lib/rlights.h" // raylib light implementation

#define CAMERA_SENSETIVITY 0.125

#define MAX_TEXTURES 16
#define GRID4x4_TEXID 0
#define GRID6x6_TEXID 1
#define GRID9x9_TEXID 2


struct state_t {

    Camera   cam;
    Vector3  player_size;
    Vector3  player_velocity;
    float    player_walkspeed;
    float    player_run_mult; // if shift is held down while walking walkspeed is multiplied with this number.
    float    player_jump_force;
    float    player_mass;
    float    player_gravity;
    int      player_onground;

    Light   lights[MAX_LIGHTS];
    Shader  light_shader;
    unsigned int num_lights;

    Texture tex[MAX_TEXTURES];
    unsigned int num_textures;

};

// used for checking collisions
struct box_t {
    Vector3 pos;
    Vector3 size;
};




#endif
