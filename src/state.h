#ifndef GSTATE_H
#define GSTATE_H


#include <rcamera.h> // raylib camera

#include "lib/rlights.h" // raylib light implementation

#define CAMERA_SENSETIVITY 0.125

#define MAX_TEXTURES 16
#define NONE_TEXID -1
#define GRID4x4_TEXID 0
#define GRID6x6_TEXID 1
#define GRID9x9_TEXID 2
#define ENEMY_0_TEXID 3
#define GUN_0_TEXID 4

#include "player.h"
#include "object.h"
#include "enemy.h"


struct state_t {

    float dt; // previous frame time.

    struct player_t player;

    Light   lights[MAX_LIGHTS];
    Shader  light_shader;
    unsigned int num_lights;

    Texture tex[MAX_TEXTURES];
    unsigned int num_textures;

    int draw_debug;


    struct obj_t* objects;
    size_t objarray_size;
    size_t num_objects;


    struct enemy_t* enemies;
    size_t enemyarray_size;
    size_t num_enemies;


};




#endif
