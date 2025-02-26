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
#define BLOOM_TRESHOLD_SHADER 4
#define MAX_SHADERS 8
// ...
 
// Particle systems.
#define MAX_PSYSTEMS 2
// ...


// Uniform locations for fragment shaders
// (index in 'fs_unilocs' array)
#define POSTPROCESS_TIME_FS_UNILOC 0
#define POSTPROCESS_SCREENSIZE_FS_UNILOC 1
#define POSTPROCESS_PLAYER_HEALTH_FS_UNILOC 2
#define PROJECTILES_PSYSTEM_COLOR_FS_UNILOC 3
#define PROJECTILE_POSTPROCESS_SCREENSIZE_FS_UNILOC 4
#define MAX_FS_UNILOCS 5



#include "enemies/enemy_lvl0.h"

#define MAX_ENTITIES 32

#define ENTWEAPON_LVL0 0
#define ENTWEAPON_LVL1 1
#define MAX_ENTITY_WEAPONS 2



// Game state "gst".
struct state_t {
    float time;
    float dt; // Previous frame time.
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


    struct entity_t entities[MAX_ENTITIES];
    size_t num_entities;

    struct weapon_t entity_weapons[MAX_ENTITY_WEAPONS];
    size_t num_entity_weapons;

    int scrn_w; // Screen width
    int scrn_h; // Screen height

    int rseed; // Seed for randomgen functions.
    int debug;


    // Everything is rendered to this texture
    // and then post processed.
    RenderTexture2D env_render_target;

    // Bloom treshold is written here.
    // when post processing. bloom is aplied and mixed into 'env_render_target' texture
    RenderTexture2D bloomtreshold_target;

};

void state_update_shader_uniforms(struct state_t* gst);
void state_update_frame(struct state_t* gst);

// Render everything to 'env_render_target'
// and post process it later.
void state_render_environment(struct state_t* gst);



#endif
