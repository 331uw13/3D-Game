#ifndef GSTATE_H
#define GSTATE_H

#include <raylib.h>
#include <rcamera.h> // raylib camera


#define CAMERA_SENSETIVITY 0.00125

// Index for textures.
#define NONE_TEXID -1
#define GRID4x4_TEXID 0
#define GRID6x6_TEXID 1
#define GRID9x9_TEXID 2
#define GUN_0_TEXID 3
#define ENEMY_LVL0_TEXID 4

#define MAX_TEXTURES 16



#include "light.h"
#include "player.h"
#include "object.h"
#include "psystem.h"
#include "terrain.h"
#include "entity.h"

// shaders.
#define DEFAULT_SHADER 0
#define POSTPROCESS_SHADER 1
#define PROJECTILES_PSYSTEM_SHADER 2
#define MAX_SHADERS 6
// ...

// particle systems.
#define MAX_PSYSTEMS 2
#define PSYS_ENEMYHIT 0
// ...


// uniform locations (index) for fragment shaders
#define POSTPROCESS_TIME_FS_UNILOC 0
#define POSTPROCESS_SCREENSIZE_FS_UNILOC 1
#define POSTPROCESS_PLAYER_HEALTH_FS_UNILOC 2
#define PROJECTILES_PSYSTEM_COLOR_FS_UNILOC 3
#define MAX_FS_UNILOCS 4



#include "enemies/enemy_lvl0.h"

#define MAX_ENTITIES 32

#define ENTWEAPON_LVL0 0
#define ENTWEAPON_LVL1 1
#define MAX_ENTITY_WEAPONS 2


// Game state.
struct state_t {
    float time;
    float dt; // previous frame time.
    struct player_t player;


    struct light_t normal_lights[MAX_NORMAL_LIGHTS];
    size_t num_normal_lights;

    // Projectile lights are created into 'weapon projectile structure'
    // NOT into 'state lights'
    size_t num_projectile_lights;


    Shader shaders[MAX_SHADERS];
    int    fs_unilocs[MAX_FS_UNILOCS];

    Texture       textures[MAX_TEXTURES];
    unsigned int  num_textures;

    struct psystem_t psystems[MAX_PSYSTEMS];
    struct terrain_t terrain;

    int draw_debug; // <- TODO.

    struct obj_t* objects;
    size_t objarray_size;
    size_t num_objects;

    struct entity_t entities[MAX_ENTITIES];
    size_t num_entities;

    struct weapon_t entity_weapons[MAX_ENTITY_WEAPONS];
    size_t num_entity_weapons;

    int rseed; // seed for randomgen functions.
};


struct enemyhit_psys_extra_t {

    Vector3 spawn_position;
    Vector3 proj_direction;

};



#endif
