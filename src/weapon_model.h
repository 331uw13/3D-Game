#ifndef PLAYER_WEAPON_MODEL_H
#define PLAYER_WEAPON_MODEL_H

#include "weapon.h"

// 'weapon.h' Holds some stats about a weapon.
// 'weapon_model.' Holds info about the model so player can have different weapons.

// Only player and items uses this.
// Enemies dont need it.


#define WMODEL_ASSAULT_RIFLE_0  0
#define WMODEL_SNIPER_RIFLE_0   1
#define MAX_WEAPON_MODELS 2

#define WMODEL_LOW_GRAVITY_FRICTION 0.95   // Used when aiming and inspecting weapon.
#define WMODEL_HIGH_GRAVITY_FRICTION 0.92  // Used when changing from rest position to aiming


struct weapon_model_t {
    int incomplete;

    Model model;
    struct weapon_t stats;

    int has_scope;
    float firerate;
    float firerate_timer;
    
    Vector3 aim_offset;
    Vector3 rest_offset;
    Vector3 rest_rotation;
    Vector3 inspect_offset;
    Vector3 inspect_rotation;
    Vector3 energy_light_offset;

    float prjfx_offset;
 
    int   recoil_anim_done;
    float recoil_anim_value; // "Interpolation"
    float recoil_anim_timer;
    float recoil_anim_kickback;

    float recoil;

    int item_info_index;


    // Used to add some movement for the weapon model
    // When player is holding it.
    Vector3 gravity_offset;
    Vector3 gravity_velocity;
    Vector3 render_offset;
    float gravity_friction;
};


void delete_weapon_model(struct state_t* gst, struct weapon_model_t* weapon_model);

void load_weapon_model(
        struct state_t* gst,
        int weapon_model_index, // Index in 'gst->weapon_models' array.
        const char* config_file_path,
        const char* weapon_name,
        const char* weapon_desc
        );

void render_weapon_model(struct state_t* gst, struct weapon_model_t* weapon_model, Matrix transform);


// This is for fine tuning the offsets.
void use_weapon_model_test_offsets(struct state_t* gst, struct weapon_model_t* weapon_model);



#endif
