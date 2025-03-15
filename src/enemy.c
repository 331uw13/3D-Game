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


struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int mood,
        Model* modelptr,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size,
        Vector3 hitbox_position,
        float target_range,
        float firerate,
        void(*update_callback)(struct state_t*, struct enemy_t*),
        void(*render_callback)(struct state_t*, struct enemy_t*),
        void(*death_callback)(struct state_t*, struct enemy_t*),
        void(*spawn_callback)(struct state_t*, struct enemy_t*),
        void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/)
){

    struct enemy_t* entptr = NULL;

    if((gst->num_enemies+1) >= MAX_ALL_ENEMIES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Max enemies reached.\033[0m\n",
                __func__);
        goto error;
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

    entptr = &gst->enemies[gst->num_enemies];
    entptr->type = enemy_type;
    entptr->index = gst->num_enemies;

    entptr->enabled = 1;
    entptr->modelptr = modelptr;

    entptr->update_callback = update_callback;
    entptr->render_callback = render_callback;
    entptr->death_callback = death_callback;
    entptr->spawn_callback = spawn_callback;

    entptr->position = initial_position; 
    entptr->hitbox_size = hitbox_size;
    entptr->hitbox_position = hitbox_position;
    entptr->health = max_health;
    entptr->max_health = max_health;

    entptr->knockback_velocity = (Vector3){0};
    entptr->stun_timer = 0.0;
    entptr->max_stun_time = 0.0;

    entptr->Q_prev   = QuaternionIdentity();
    entptr->Q_target = QuaternionIdentity();
    entptr->angle_change = 0.0;
    entptr->rotation = (Vector3){ 0.0, 0.0, 0.0 };
    
    entptr->target_range = target_range;
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
    gst->num_enemies++;


    entptr->firerate_timer = 0.0;
    entptr->firerate = firerate;



    entptr->spawn_callback(gst, entptr);

    
error:
    return entptr;
}


void update_enemy(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->enabled) {
        return;
    }
    if(ent->update_callback) {
        ent->dist_to_player = Vector3Distance(gst->player.position, ent->position);
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

    if(ent->hit_callback) {
        ent->hit_callback(gst, ent, hit_position, hit_direction);
    }

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
                create_enemy(gst,
                        enemy_type,
                        mood,
                        &gst->enemy_models[enemy_type],
                        &gst->psystems[ENEMY_LVL0_WEAPON_PSYS],
                        &gst->enemy_weapons[enemy_type],
                        max_health,
                        position,
                        (Vector3){ 4.0, 4.0, 4.0 }, /* Hitbox size */
                        (Vector3){ 0.0, 2.0, 0.0 }, /* Hitbox position offset */
                        150.0, /* Target range */
                        0.4,   /* Firerate */
                        enemy_lvl0_update,
                        enemy_lvl0_render,
                        enemy_lvl0_death,
                        enemy_lvl0_created,
                        enemy_lvl0_hit
                        );
            }
            break;

        default:
            fprintf(stderr, "\033[31m(ERROR) '%s': Invalid enemy type '%i'\033[0m\n",
                    __func__, enemy_type);
            break;
    }
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


int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent) {
    return (!_is_terrain_blocking_view(gst, ent) && (ent->dist_to_player <= ent->target_range));
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
