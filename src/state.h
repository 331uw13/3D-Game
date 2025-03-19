#ifndef GSTATE_H
#define GSTATE_H

#define GRAPHICS_API_OPENGL_43

#include <raylib.h>
#include <rcamera.h> // raylib camera

#include "light.h"
#include "player.h"
#include "psystem.h"
#include "terrain.h"
#include "enemy.h"
#include "item.h"
#include "gui.h"

#define DEF_SCRN_W 1500
#define DEF_SCRN_H 800

#define TARGET_FPS 500
#define CAMERA_SENSETIVITY 0.00125
#define MAX_VOLUME_DIST 720 // How far away can player hear sounds.?

#define GRAVITY_CONST 500
#define FONT_SPACING 1.0

// Index for 'textures'.
#define NONE_TEXID -1 // TODO: remove this <-
#define GRID4x4_TEXID 0
#define GRID6x6_TEXID 1
#define GRID9x9_TEXID 2
#define GUN_0_TEXID 3
#define ENEMY_LVL0_TEXID 4
#define PLAYER_ARMS_TEXID 5
#define CRITICALHIT_TEXID 6
#define TREEBARK_TEXID 7
#define LEAF_TEXID 8
#define ROCK_TEXID 9
#define MOSS_TEXID 10
#define GRASS_TEXID 11
#define GUNFX_TEXID 12
#define APPLE_INV_TEXID 13
#define APPLE_TEXID 14
#define METALPIECE_INV_TEXID 15
#define METALPIECE_TEXID 16
#define MAX_TEXTURES 17
// ...


// Index for 'sounds'
#define PLAYER_GUN_SOUND 0
#define ENEMY_HIT_SOUND_0 1
#define ENEMY_HIT_SOUND_1 2
#define ENEMY_HIT_SOUND_2 3
#define ENEMY_GUN_SOUND 4
#define PRJ_ENVHIT_SOUND 5
#define PLAYER_HIT_SOUND 6
#define ENEMY_EXPLOSION_SOUND 7
#define MAX_SOUNDS 8
//...


// Index for 'shaders'
#define DEFAULT_SHADER 0
#define POSTPROCESS_SHADER 1
#define BLOOM_TRESHOLD_SHADER 2
#define WDEPTH_SHADER 3
#define PRJ_ENVHIT_PSYS_SHADER 4
#define BASIC_WEAPON_PSYS_SHADER 5
#define FOLIAGE_SHADER 6
#define FOG_PARTICLE_SHADER 7
#define WATER_SHADER 8
#define GUNFX_SHADER 9
#define ENEMY_GUNFX_SHADER 10
#define MAX_SHADERS 11
// ...
 

// Player's weapon particle system is stored in player struct.
// Enemies have pointers into global particle systems and weapons.

// Global particle systems:
#define PLAYER_PRJ_ENVHIT_PSYS 0
#define ENEMY_PRJ_ENVHIT_PSYS 1
#define ENEMY_LVL0_WEAPON_PSYS 2
#define ENEMY_HIT_PSYS 3 
#define FOG_EFFECT_PSYS 4
#define PLAYER_HIT_PSYS 5
#define ENEMY_EXPLOSION_PSYS 6
#define ENEMY_EXPLOSION_PART2_PSYS 7 // For extra effect.
#define WATER_SPLASH_PSYS 8
#define ENEMY_GUNFX_PSYS 9
#define ENEMY_PRJ_ENVHIT_PART2_PSYS 10   // For extra effect.
#define PLAYER_PRJ_ENVHIT_PART2_PSYS 11 // For extra effect.
#define MAX_PSYSTEMS 12
// ...


// TODO: Clean this up.
// Uniform locations for fragment shaders
// (index in 'fs_unilocs' array)
#define POSTPROCESS_TIME_FS_UNILOC 0
#define POSTPROCESS_SCREENSIZE_FS_UNILOC 1
#define POSTPROCESS_PLAYER_HEALTH_FS_UNILOC 2
#define POSTPROCESS_CAMTARGET_FS_UNILOC 3
#define POSTPROCESS_CAMPOS_FS_UNILOC 4
#define PROJECTILE_POSTPROCESS_SCREENSIZE_FS_UNILOC 5
#define FOLIAGE_SHADER_TIME_FS_UNILOC 6
#define WATER_SHADER_TIME_FS_UNILOC 7
#define GUNFX_SHADER_COLOR_FS_UNILOC 8
#define ENEMY_GUNFX_SHADER_COLOR_FS_UNILOC 9
#define MAX_FS_UNILOCS 10
// ...


// Normal lights:
#define SUN_NLIGHT 0
#define PLAYER_GUN_NLIGHT 1
// ...


#include "enemies/enemy_lvl0.h"



#define ENEMY_LVL0_WEAPON 0
#define ENEMY_LVL1_WEAPON 1
#define MAX_ENEMY_WEAPONS 2


#define MAX_ALL_ENEMIES 64 // Total max enemies.
#define MAX_ENEMY_MODELS 1



#define MAX_RENDER_CRITHITS 8

#define SUN_LIGHT_ID 0
#define PLAYER_GUN_LIGHT_ID 1


// Critical hit marker.
struct crithit_marker_t {
    Vector3 position;
    float   lifetime; // Scale smaller overtime.
    int     visible;
    float   dst; // Distance to player. Used for sorting to fix alpha blending.
};


// Enemy spawn settings for each type.
struct spawn_system_t {
    int   max_in_spawn_radius [MAX_ENEMY_TYPES];  // How many can spawn in spawn radius?
    int   max_in_world        [MAX_ENEMY_TYPES];  // How many can be spawned in world total?
    float spawn_radius        [MAX_ENEMY_TYPES];  // Spawn radius around player position.
    
    float spawn_timers_max    [MAX_ENEMY_TYPES];  // How long to wait until more can be tried to spawn?
    float spawn_timers        [MAX_ENEMY_TYPES];  // Time elapsed.
    
    int num_spawns_min [MAX_ENEMY_TYPES]; // Min number of enemies to spawn
    int num_spawns_max [MAX_ENEMY_TYPES]; // Max number of enemies to spawn
};


// Game state "gst".
struct state_t {
    float time;
    float dt; // Previous frame time.
    struct player_t player;
    Font font;


    unsigned int lights_ubo;
    unsigned int prj_lights_ubo;
    
    size_t num_prj_lights;


    Shader shaders[MAX_SHADERS];
    int    fs_unilocs[MAX_FS_UNILOCS];

    Texture       textures[MAX_TEXTURES];
    unsigned int  num_textures;

    
    struct psystem_t psystems[MAX_PSYSTEMS];
    struct terrain_t terrain;

    struct spawn_system_t spawnsys;
    Model  enemy_models[MAX_ENEMY_MODELS];
    struct enemy_t enemies[MAX_ALL_ENEMIES];
    size_t num_enemies;
   


    struct weapon_t enemy_weapons[MAX_ENEMY_WEAPONS];
    size_t num_enemy_weapons;


    struct item_t items[MAX_ALL_ITEMS];
    Model         item_models[MAX_ITEM_MODELS];
    size_t        num_items;

    float         natural_item_spawn_timers[MAX_ITEM_TYPES];

    int scrn_w; // Screen width
    int scrn_h; // Screen height

    int rseed; // Seed for randomgen functions.
    int debug;

    struct crithit_marker_t crithit_markers[MAX_RENDER_CRITHITS];
    size_t num_crithit_markers;
    float  crithit_marker_maxlifetime;


    int xp_value_add;
    float xp_update_timer;


    int has_audio;
    Sound sounds[MAX_SOUNDS];

    // Everything is rendered to this texture
    // and then post processed.
    RenderTexture2D env_render_target;

    // Bloom treshold is written here.
    // when post processing. bloom is aplied and mixed into 'env_render_target' texture
    RenderTexture2D bloomtreshold_target;

    // (NOT CURRENTLY USED)
    RenderTexture2D depth_texture;


    int running;
    int menu_open;
};


void state_update_shader_uniforms(struct state_t* gst);
void state_render_environment(struct state_t* gst);
void state_update_frame(struct state_t* gst);


// Initialization.
void state_setup_all_shaders(struct state_t* gst);
void state_setup_all_weapons(struct state_t* gst);
void state_setup_all_psystems(struct state_t* gst);
void state_setup_all_textures(struct state_t* gst);
void state_setup_all_sounds(struct state_t* gst);
void state_setup_all_enemy_models(struct state_t* gst);
void state_setup_all_item_models(struct state_t* gst);

// Free up memory.
void state_delete_all_shaders(struct state_t* gst);
void state_delete_all_psystems(struct state_t* gst);
void state_delete_all_sounds(struct state_t* gst);
void state_delete_all_textures(struct state_t* gst);
void state_delete_all_enemy_models(struct state_t* gst);
void state_delete_all_item_models(struct state_t* gst);


// Misc.
void state_add_crithit_marker(struct state_t* gst, Vector3 position);



#endif
