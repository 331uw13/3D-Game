#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "enemy.h"
#include "util.h"

#include "enemies/enemy_lvl0.h"



struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int texture_id,
        const char* model_filepath,
        const char* broken_model_filepath,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size,
        Vector3 hitbox_position,
        float target_range,
        float firerate
){
    struct enemy_t* entptr = NULL;

    if((gst->num_enemies+1) >= MAX_ENEMIES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Max enemies reached.\033[0m\n",
                __func__);
        goto error;
    }

    if(!FileExists(model_filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Model file path not found: '%s'\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    if(texture_id >= MAX_TEXTURES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid texture id: %i for (%s)\033[0m\n",
                __func__, texture_id, model_filepath);
        goto error;
    }

    entptr = &gst->enemies[gst->num_enemies];
    entptr->type = enemy_type;

    entptr->position = initial_position; 
    entptr->hitbox_size = hitbox_size;
    entptr->hitbox_position = hitbox_position;
    entptr->health = max_health;
    entptr->max_health = max_health;

    entptr->knockback_velocity = (Vector3){0};
    entptr->hit_direction = (Vector3){0};
    entptr->rotation_from_hit = (Vector3){0};
    entptr->stun_timer = 0.0;
    entptr->max_stun_time = 0.0;

    entptr->Q0 = QuaternionIdentity();
    entptr->Q1 = QuaternionIdentity();
    entptr->angle_change = 0.0;
    entptr->forward_angle = 0.0;
    
    entptr->target_range = target_range;
    entptr->has_target = 0;
    entptr->body_matrix = MatrixIdentity();
    entptr->gun_index = 0;
    entptr->index = gst->num_enemies;

    entptr->alive = 1;
    entptr->weaponptr = weaponptr;
    entptr->weapon_psysptr = weapon_psysptr;

    entptr->travel = (struct enemy_travel_t) {
        .start = (Vector3){0},
        .dest  = (Vector3){0},
        .travelled = 0.0,

        // Next enemy update will set start and dest values
        // If travelling is enabled.
        .dest_reached = 1
    };


    entptr->model = LoadModel(model_filepath);
    entptr->model.transform 
        = MatrixTranslate(initial_position.x, initial_position.y, initial_position.z);

    entptr->model.materials[0] = LoadMaterialDefault();
    entptr->model.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    entptr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[texture_id];


    gst->num_enemies++;

    printf("(INFO) '%s': Enemy (Index:%li). Model filepath:\"%s\"\n",
            __func__, entptr->index, model_filepath);

    entptr->firerate_timer = 0.0;
    entptr->firerate = firerate;



    load_enemy_broken_model(entptr, broken_model_filepath);

    switch(entptr->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_created(gst, entptr);
            break;

        // ...
    }

error:
    return entptr;
}


void load_enemy_broken_model(struct enemy_t* ent, const char* broken_model_filepath) {
    ent->broken_matrices = NULL;
    ent->broken_mesh_velocities = NULL;
    ent->broken_model = (Model) { 0 };

    if(!broken_model_filepath) {
        printf("\033[36m(WARNING) '%s': No \"broken\" model filepath\033[0m\n",
                __func__);
        return;
    }

    if(!FileExists(broken_model_filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': '%s' Not found.\033[0m\n",
                __func__, broken_model_filepath);
        return;
    }


    ent->broken_model = LoadModel(broken_model_filepath);
    
    // Allocate memory for model's mesh matrices.
    ent->broken_matrices = malloc(ent->broken_model.meshCount * sizeof *ent->broken_matrices);

    // Allocate memory for velocities.
    ent->broken_mesh_velocities = malloc(ent->broken_model.meshCount * sizeof *ent->broken_mesh_velocities);
  
    // Allocate memory for rotations.
    ent->broken_mesh_rotations = malloc(ent->broken_model.meshCount * sizeof *ent->broken_mesh_rotations);

    for(int i = 0; i < ent->broken_model.meshCount; i++) {
        ent->broken_matrices[i] = MatrixIdentity();
        ent->broken_mesh_velocities[i] = (Vector3) { 0.0, 0.0, 0.0 };
        ent->broken_mesh_rotations[i] = (Vector3) { 0.0, 0.0, 0.0 };
    }


    ent->broken_model_despawn_timer = 0.0;
    ent->broken_model_despawn_maxtime = 10.0;
    ent->broken_model_despawned = 0;

}


void delete_enemy(struct enemy_t* ent) {
    UnloadModel(ent->model);
    ent->health = 0;
    ent->weapon_psysptr = NULL;
    ent->weaponptr = NULL;

    if(IsModelValid(ent->broken_model)) {
        UnloadModel(ent->broken_model);

        if(ent->broken_matrices) {
            free(ent->broken_matrices);
            ent->broken_matrices = NULL;
        }

        if(ent->broken_mesh_velocities) {
            free(ent->broken_mesh_velocities);
            ent->broken_mesh_velocities = NULL;
        }

        if(ent->broken_mesh_rotations) {
            free(ent->broken_mesh_rotations);
            ent->broken_mesh_rotations = NULL;
        }
    }
}


void update_enemy(struct state_t* gst, struct enemy_t* ent) {
    
    if(!ent->alive && !ent->broken_model_despawned) {
        // Enemy has died so update its matrix transforms for "broken" model
        update_enemy_broken_matrices(gst, ent);
    }

    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_update(gst, ent);
            break;


        // ...
    }

}

void render_enemy(struct state_t* gst, struct enemy_t* ent) {
    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_render(gst, ent);
            break;


        // ...
    }


}


    
void enemy_hit(struct state_t* gst, struct enemy_t* ent, struct weapon_t* weapon, 
        Vector3 hit_position, Vector3 hit_direction) {

    int was_critical_hit = 0;
    ent->health -= get_weapon_damage(weapon, &was_critical_hit);

    if(was_critical_hit) {
        state_add_crithit_marker(gst, hit_position);
    }

    if(gst->has_audio) {
        int audio_i = ENEMY_HIT_SOUND_0 + GetRandomValue(0, 2); // 3 enemy hit sounds.

        // Set volume based on distance.
        SetSoundVolume(gst->sounds[audio_i], get_volume_dist(gst->player.position, ent->position));
        PlaySound(gst->sounds[audio_i]);
    }

    if(ent->health <= 0.001) {
        ent->health = 0.0;
        ent->alive = 0;
        enemy_death(gst, ent);
        return;
    }

    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_hit(gst, ent, hit_position, hit_direction);
            break;


        // ...
    }

}

void enemy_death(struct state_t* gst, struct enemy_t* ent) {

    // Update broken model matrices to last known body position, if they exist
    // And decide random velocities.
    if(IsModelValid(ent->broken_model) && ent->broken_matrices) {
        for(int i = 0; i < ent->broken_model.meshCount; i++) {
            ent->broken_matrices[i] = ent->body_matrix;
            ent->broken_matrices[i].m13 += 1.0;

            const float r = 80.0;
            ent->broken_mesh_velocities[i] = (Vector3) {
                RSEEDRANDOMF(-r, r),
                RSEEDRANDOMF(r, r*1.5),
                RSEEDRANDOMF(-r, r),
            };

        }
    }

    add_particles(
            gst,
            &gst->psystems[ENEMY_EXPLOSION_PSYS],
            GetRandomValue(16, 32),
            ent->position,
            (Vector3){0},
            NULL, NO_EXTRADATA
            );

    SetSoundVolume(gst->sounds[ENEMY_EXPLOSION_SOUND], get_volume_dist(gst->player.position, ent->position));
    SetSoundPitch(gst->sounds[ENEMY_EXPLOSION_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.3));
    PlaySound(gst->sounds[ENEMY_EXPLOSION_SOUND]);

    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_death(gst, ent);
            break;


        // ...
    }
   
}


static int _is_terrain_blocking_view(struct state_t* gst, struct enemy_t* ent) {
    int result = 0;

    Vector3 ent_direction = Vector3Normalize(Vector3Subtract(gst->player.position, ent->position));
    Vector3 ray_position = (Vector3){
        ent->position.x,
        ent->position.y + 3,
        ent->position.z
    };

    // Move 'ray_position' towards player
    // and cast ray from 'terrain.highest_point' at ray_position.X and Z.
    // to see if the hit Y position is bigger than ray Y position, if so terrain was hit.
    // this is not perfect but will do for now i guess.

    Vector3 step = Vector3Scale(ent_direction, 3.0);
    const int max_steps = 255;
    for(int i = 0; i < max_steps; i++) {
        
        ray_position = Vector3Add(ray_position, step);

        RayCollision t_hit = raycast_terrain(&gst->terrain, ray_position.x, ray_position.z);
        if(t_hit.point.y >= ray_position.y) {
            result = 1;
            break;
        }
        
        if(Vector3Distance(ray_position, gst->player.position) < 4.0) {
            break;
        }
    }

    return result;
}

int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent) {
    const float dst2player = Vector3Distance(gst->player.position, ent->position);
    return (!_is_terrain_blocking_view(gst, ent) && (dst2player <= ent->target_range));
}

BoundingBox get_enemy_boundingbox(struct enemy_t* ent) {
    return (BoundingBox) {
        (Vector3) {
            // Minimum box corner
            (ent->position.x + ent->hitbox_position.x) - ent->hitbox_size.x/2,
            (ent->position.y + ent->hitbox_position.y) - ent->hitbox_size.y/2,
            (ent->position.z + ent->hitbox_position.z) - ent->hitbox_size.z/2
        },
        (Vector3) {
            // Maximum box corner
            (ent->position.x + ent->hitbox_position.x) + ent->hitbox_size.x/2,
            (ent->position.y + ent->hitbox_position.y) + ent->hitbox_size.y/2,
            (ent->position.z + ent->hitbox_position.z) + ent->hitbox_size.z/2
        }
    };
}


void update_enemy_broken_matrices(struct state_t* gst, struct enemy_t* ent) {

    const float dampen = pow(0.5, gst->dt);
    const float rt_time = gst->time * 3.0; // Rotation time.

    for(int i = 0; i < ent->broken_model.meshCount; i++) {
        float x = ent->broken_matrices[i].m12;
        float y = ent->broken_matrices[i].m13;
        float z = ent->broken_matrices[i].m14;
    
        Vector3* vel = &ent->broken_mesh_velocities[i];

        // Check collision with terrain.
        RayCollision ray = raycast_terrain(&gst->terrain, x, z);

        float d = dampen;

        if(ray.point.y < y) {
            y += vel->y * gst->dt;
            vel->y -= (500*0.2) * gst->dt;

            ent->broken_mesh_rotations[i].x = rt_time * 1.524;
            ent->broken_mesh_rotations[i].y = rt_time * 0.165;
            ent->broken_mesh_rotations[i].z = rt_time * 0.285;
        }
        else {
            d *= 0.5;
        }
        
        x += vel->x * gst->dt;
        z += vel->z * gst->dt;
        
        vel->x *= d;
        vel->z *= d; 

        Matrix rotation = MatrixRotateXYZ(ent->broken_mesh_rotations[i]);
        Matrix translation = MatrixTranslate(x, y, z);
        ent->broken_matrices[i] = MatrixMultiply(rotation, translation);
    }
}

