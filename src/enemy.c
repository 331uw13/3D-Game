#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "enemy.h"
#include "util.h"

#include "enemies/enemy_lvl0.h"


static int _is_terrain_blocking_view(struct state_t* gst, struct enemy_t* ent) {
    int result = 0;

    Vector3 ent_direction = Vector3Normalize(Vector3Subtract(gst->player.position, ent->position));
    Vector3 ray_position = (Vector3){
        ent->position.x,
        ent->position.y + 3.0,
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


int load_enemy_model(struct state_t* gst, u32 enemy_type, const char* model_filepath, int texture_id) {
    int result = 0;

    if(!FileExists(model_filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Model filepath not found: \"%s\"\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    if(enemy_type > MAX_ALL_ENEMIES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid enemy type \"%i\"\033[0m\n",
                __func__, enemy_type);
        goto error;
    }

    if(enemy_type >= MAX_ENEMY_MODELS) {
        fprintf(stderr, 
                "\033[31m(ERROR) '%s': 'enemy_models' array doesnt have enough space for all models.\033[0m\n",
                __func__);
        goto error;
    }

    SetTraceLogLevel(LOG_ALL);
    Model* model = &gst->enemy_models[enemy_type];
    *model = LoadModel(model_filepath);

    if(!IsModelValid(*model)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to load enemy model \"%s\"\033[0m\n",
                __func__, model_filepath);
        goto error;
    }


    // TODO: make this support more textures later.
   
    for(int i = 0; i < model->materialCount; i++) {
        model->materials[i] = LoadMaterialDefault();
        model->materials[i].shader = gst->shaders[DEFAULT_SHADER];
        model->materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[texture_id];
    }
error:
    SetTraceLogLevel(LOG_NONE);
    return result;
}

struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int mood,
        Model* modelptr,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        float target_range,
        float target_fov,
        float firerate,
        void(*update_callback)(struct state_t*, struct enemy_t*),
        void(*render_callback)(struct state_t*, struct enemy_t*),
        void(*death_callback)(struct state_t*, struct enemy_t*),
        void(*spawn_callback)(struct state_t*, struct enemy_t*),
        void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/)
){


    struct enemy_t* entptr = NULL;
    size_t entptr_index = gst->num_enemies;

    if(gst->num_enemies+1 >= MAX_ALL_ENEMIES) {
        
        // Try to search for available position.
        size_t i = 0;
        for(; i < MAX_ALL_ENEMIES; i++) {
            if(!gst->enemies[i].alive) {
                entptr_index = i;
                break;
            }
        }

        if(i == MAX_ALL_ENEMIES) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Max enemies reached.\033[0m\n",
                    __func__);
            goto error;
        }
    }

    if(!modelptr) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Pointer to enemy model seems to be NULL\033[0m\n",
                __func__);
        goto error;
    }

    if(!update_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy(%li) is missing update callback\033[0m\n",
                __func__, entptr->index);
        goto error;
    }
    if(!render_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy(%li) is missing render callback\033[0m\n",
                __func__, entptr->index);
        goto error;
    }
    if(!death_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy(%li) is missing death callback\033[0m\n",
                __func__, entptr->index);
        goto error;
    }
    if(!spawn_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy(%li) is missing spawn callback\033[0m\n",
                __func__, entptr->index);
        goto error;
    }
    if(!hit_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy(%li) is missing hit callback\033[0m\n",
                __func__, entptr->index);
        goto error;
    }

    entptr = &gst->enemies[entptr_index];
    entptr->type = enemy_type;
    entptr->index = entptr_index;

    entptr->num_hitboxes = 0;
    entptr->enabled = 1;
    entptr->modelptr = modelptr;

    entptr->update_callback = update_callback;
    entptr->render_callback = render_callback;
    entptr->death_callback = death_callback;
    entptr->spawn_callback = spawn_callback;

    entptr->position = initial_position; 
    entptr->health = max_health;
    entptr->max_health = max_health;

    entptr->knockback_velocity = (Vector3){0};
    entptr->stun_timer = 0.0;
    entptr->max_stun_time = 0.0;

    entptr->Q_prev   = QuaternionIdentity();
    entptr->Q_target = QuaternionIdentity();
    entptr->angle_change = 0.0;
    entptr->rotation = (Vector3){ 0.0, 0.0, 0.0 };
   
    entptr->Q_rnd_prev = QuaternionIdentity();
    entptr->Q_rnd_target = QuaternionIdentity();
    entptr->rnd_angle_change = 0.0;

    entptr->target_range = target_range;
    entptr->target_fov = CLAMP(target_fov, 0, 180.0);
    entptr->gun_index = 0;
    entptr->has_target = 0;
    entptr->mood = mood;

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

    for(int i = 0; i < ENEMY_MAX_MATRICES; i++) {
        entptr->matrix[i] = MatrixIdentity();
    }


    printf("(INFO) '%s': Enemy (Index:%li)\n",
            __func__, entptr->index);
  

    entptr->accuracy_modifier = 0.0;
    entptr->firerate_timer = 0.0;
    entptr->firerate = firerate;

    gst->num_enemies++;
    if(gst->num_enemies >= MAX_ALL_ENEMIES) {
        gst->num_enemies = MAX_ALL_ENEMIES;
    } 

error:
    return entptr;
}


void update_enemy(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->enabled) {
        return;
    }
    if(ent->update_callback) {
        ent->dist_to_player = Vector3Distance(gst->player.position, ent->position);
        ent->firerate_timer += gst->dt;
        ent->update_callback(gst, ent);
    }
}

void render_enemy(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->enabled) {
        return;
    }
    if(ent->dist_to_player > RENDER_DISTANCE) {
        return;
    }
    if(ent->render_callback) {
        ent->render_callback(gst, ent);
    }
}


void enemy_hit(
        struct state_t* gst,
        struct enemy_t* ent,
        struct weapon_t* weapon, 
        float damage_mult,
        Vector3 hit_position,
        Vector3 hit_direction
){

    int was_critical_hit = 0;
    float damage = get_weapon_damage(weapon, &was_critical_hit) * damage_mult;
    ent->health -= damage;


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

    if(ent->hit_callback) {
        ent->hit_callback(gst, ent, hit_position, hit_direction);
    }

    printf("'%s': %0.2f | Health: %0.2f\n", __func__, damage, ent->health);
}

void enemy_death(struct state_t* gst, struct enemy_t* ent) {
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

    if(ent->death_callback) {
        ent->death_callback(gst, ent);
    }
}

void enemy_add_hitbox(
        struct enemy_t* ent, 
        Vector3 hitbox_size,
        Vector3 hitbox_offset,
        float damage_multiplier
){
    if(ent->num_hitboxes+1 >= ENEMY_MAX_HITBOXES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Trying to add too many hitboxes\033[0m\n",
                __func__);
        return;
    }

    ent->hitboxes[ent->num_hitboxes] = (struct hitbox_t) {
        .size = hitbox_size,
        .offset = hitbox_offset,
        .damage_mult = damage_multiplier,
        .id = ent->num_hitboxes
    };

    ent->num_hitboxes++;
}

void spawn_enemy(
        struct state_t* gst,
        int enemy_type,
        int max_health,
        int mood,
        Vector3 position
){

    switch(enemy_type) {
    
        case ENEMY_LVL0:
            {
                if(position.y <= gst->terrain.water_ylevel) {
                    fprintf(stderr, "\033[31m(ERROR) '%s': Enemy type '%i' cannot spawn in water.\033[0m\n",
                            __func__, enemy_type);
                    return;
                }

                struct enemy_t* ent = create_enemy(gst,
                        enemy_type,
                        mood,
                        &gst->enemy_models[enemy_type],
                        &gst->psystems[ENEMY_LVL0_WEAPON_PSYS],
                        &gst->enemy_weapons[enemy_type],
                        max_health,
                        position,
                        350.0, /* Target Range */
                        180.0, /* Target FOV */
                        0.2,   /* Firerate */
                        enemy_lvl0_update,
                        enemy_lvl0_render,
                        enemy_lvl0_death,
                        enemy_lvl0_created,
                        enemy_lvl0_hit
                        );
                if(!ent) {
                    return;
                }

                // Head hitbox.
                enemy_add_hitbox(ent,
                        (Vector3){ 13.0, 8.0, 13.0 },
                        (Vector3){ 0.0, 12.0, 0.0 },
                        1.758
                        );

                // Legs hitbox.
                enemy_add_hitbox(ent,
                        (Vector3){ 10.0, 5.0, 10.0 },
                        (Vector3){ 0.0, 3.0, 0.0 },
                        0.5
                        );

                ent->spawn_callback(gst, ent);
            }
            break;

        default:
            fprintf(stderr, "\033[31m(ERROR) '%s': Invalid enemy type '%i'\033[0m\n",
                    __func__, enemy_type);
            break;
    }
}

void delete_enemy(struct state_t* gst, size_t enemy_index) {
    if(enemy_index >= gst->num_enemies) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid index to remove enemy from.\033[0m\n",
                __func__);
        return;
    }

 

}


int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent) {
    return (!_is_terrain_blocking_view(gst, ent) && (ent->dist_to_player <= ent->target_range));
}

int player_in_enemy_fov(struct state_t* gst, struct enemy_t* ent, Matrix* body_matrix) {
    int result = 0;
    if(!body_matrix) {
        goto error;
    }

    Vector3 up     = (Vector3) { 0.0, 1.0, 0.0 };
    Vector3 right  = (Vector3) { body_matrix->m8, body_matrix->m9, body_matrix->m10 };

    Vector3 forward = Vector3CrossProduct(up, right);


    Vector3 P1 = (Vector3) {
        ent->position.x, 0, ent->position.z
    };

    Vector3 P2 = (Vector3) {
        gst->player.position.x, 0, gst->player.position.z
    };

    Vector3 D = Vector3Normalize(Vector3Subtract(P1, P2));
    float dot = Vector3DotProduct(D, forward);

    float fov = ceil(map(dot, 1.0, -1.0, 0.0, 180.0));
    result = (fov <= ent->target_fov);

error:
    return result;
}

struct hitbox_t* check_collision_hitboxes(BoundingBox* boundingbox, struct enemy_t* ent) {
    struct hitbox_t* result = NULL;

    for(size_t i = 0; i < ent->num_hitboxes; i++) {
        BoundingBox hitbox = (BoundingBox) {
            (Vector3) {
                // Minimum box corner
                (ent->position.x + ent->hitboxes[i].offset.x) - ent->hitboxes[i].size.x/2,
                (ent->position.y + ent->hitboxes[i].offset.y) - ent->hitboxes[i].size.y/2,
                (ent->position.z + ent->hitboxes[i].offset.z) - ent->hitboxes[i].size.z/2,
            },
            (Vector3) {
                // Maximum box corner
                (ent->position.x + ent->hitboxes[i].offset.x) + ent->hitboxes[i].size.x/2,
                (ent->position.y + ent->hitboxes[i].offset.y) + ent->hitboxes[i].size.y/2,
                (ent->position.z + ent->hitboxes[i].offset.z) + ent->hitboxes[i].size.z/2,
            }
        };

        if(CheckCollisionBoxes(*boundingbox, hitbox)) {
            result = &ent->hitboxes[i];
            break;
        }
    }


    return result;
}

