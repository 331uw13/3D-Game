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
#include "psystem.h"
#include "terrain.h"

// shaders.
#define MAX_SHADERS 4
#define DEFAULT_SHADER 0
#define PLAYER_PROJECTILE_SHADER 1
#define ENEMY_HIT_PSYS_SHADER 2
// ...

// particle systems.
#define MAX_PSYSTEMS 2
#define PSYS_ENEMYHIT 0
// ...


// uniform locations for fragment shaders
#define MAX_FS_UNILOCS 4
#define PLAYER_PROJECTILE_EFFECTSPEED_FS_UNILOC 0



struct state_t {

    float dt; // previous frame time.
    struct player_t player;


    Light         lights[MAX_LIGHTS];
    unsigned int  num_lights;

    Light         projectile_lights[MAX_PROJECTILE_LIGHTS];
    unsigned int  next_projlight_index;
    

    Shader shaders[MAX_SHADERS];
    int    fs_unilocs[MAX_FS_UNILOCS];


    Texture       tex[MAX_TEXTURES];
    unsigned int  num_textures;

    struct psystem_t psystems[MAX_PSYSTEMS];
    
    struct terrain_t terrain;


    int draw_debug; // <- TODO.

    struct obj_t* objects;
    size_t objarray_size;
    size_t num_objects;


    struct enemy_t* enemies;
    size_t enemyarray_size;
    size_t num_enemies;


    int rseed; // seed for randomgen functions.
};


struct enemyhit_psys_extra_t {

    Vector3 spawn_position;
    Vector3 proj_direction;

};



#endif
