#include <stdio.h>
#include <stdlib.h>

#include "state/state.h"
#include "enemy.h"
#include "util.h"
#include "chunk.h"

#include "enemies/enemy_lvl0.h"
#include "enemies/enemy_lvl1.h"

#include <rlgl.h>


static int _is_terrain_blocking_view(struct state_t* gst, struct enemy_t* ent) {
    int result = 0;

    Vector3 ray_position = (Vector3){
        ent->position.x,
        ent->position.y + 10.0,
        ent->position.z
    };
    
    Vector3 ent_direction = Vector3Normalize(Vector3Subtract(gst->player.position, ray_position));

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

    if(enemy_type >= MAX_ALL_ENEMIES) {
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

    result = 1;

error:
    SetTraceLogLevel(LOG_NONE);
    return result;
}

int create_enemy_ext(
        struct state_t* gst,
        struct enemy_t* entptr,
        int enemy_type,
        int mood,
        Model* modelptr,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        int xp_gain,
        float target_range,
        float target_fov,
        float firerate,
        void(*update_callback)(struct state_t*, struct enemy_t*),
        void(*render_callback)(struct state_t*, struct enemy_t*),
        void(*death_callback)(struct state_t*, struct enemy_t*),
        void(*spawn_callback)(struct state_t*, struct enemy_t*),
        void(*hit_callback)(struct state_t*, struct enemy_t*, Vector3/*hit pos*/, Vector3/*hit dir*/,float/*knockback*/)
){
    int result = 0;

    if(!modelptr) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Pointer to enemy model seems to be NULL\033[0m\n",
                __func__);
        goto error;
    }

    if(!update_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy is missing update callback\033[0m\n",
                __func__);
        goto error;
    }
    if(!render_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy is missing render callback\033[0m\n",
                __func__);
        goto error;
    }
    if(!death_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy is missing death callback\033[0m\n",
                __func__);
        goto error;
    }
    if(!spawn_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy is missing spawn callback\033[0m\n",
                __func__);
        goto error;
    }
    if(!hit_callback) {
        fprintf(stderr, "\033[31m(ERROR) '%s' Enemy is missing hit callback\033[0m\n",
                __func__);
        goto error;
    }

    //entptr = &gst->enemies[entptr_index];
    entptr->type = enemy_type;

    entptr->num_hitboxes = 0;
    entptr->enabled = 1;
    entptr->modelptr = modelptr;

    entptr->update_callback = update_callback;
    entptr->render_callback = render_callback;
    entptr->death_callback = death_callback;
    entptr->spawn_callback = spawn_callback;
    entptr->hit_callback = hit_callback;

    entptr->xp_gain = xp_gain;
    entptr->position = initial_position; 
    entptr->health = max_health;
    entptr->max_health = max_health;

    entptr->knockback_velocity = (Vector3){0};
    entptr->time_from_hit = 99999.0;

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
    entptr->was_hit = 0;
    entptr->alive = 1;
    entptr->weaponptr = weaponptr;
    entptr->weapon_psysptr = weapon_psysptr;

    entptr->ccheck_radius = 120.0;

    entptr->time_from_target_found = 0.0;
    entptr->time_from_target_lost = 0.0;

    entptr->travel = (struct enemy_travel_t) {
        .start = entptr->position,
        .dest  = (Vector3){0},
        .travelled = 0.0,

        // Next enemy update will set start and dest values
        // If travelling is enabled.
        .dest_reached = 1
    };

    for(int i = 0; i < ENEMY_MAX_MATRICES; i++) {
        entptr->matrix[i] = MatrixIdentity();
    }


    printf("%s: ENEMY_LVL%i\n",
            __func__, entptr->type);
  

    entptr->accuracy_modifier = 0.0;
    entptr->firerate_timer = 0.0;
    entptr->firerate = firerate;
    entptr->despawn_timer = 0.0;

    result = 1;

error:
    return result;
}


void update_enemy(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->enabled) {
        return;
    }

    if(ent->dist_to_player >= ENEMY_DESPAWN_RADIUS) {
        ent->despawn_timer += gst->dt;
        if(ent->despawn_timer >= ENEMY_DESPAWN_TIME) {
            ent->alive = 0;
            ent->enabled = 0;
        }
    }
    else {
        ent->despawn_timer = 0.0;
    }

    ent->time_from_hit += gst->dt;

    if(ent->update_callback) {
        ent->dist_to_player = Vector3Distance(gst->player.position, ent->position);
        ent->firerate_timer += gst->dt;
        ent->update_callback(gst, ent);
    }

    ent->was_hit = 0;



    // Update enemy chunk if it changed.
    struct chunk_t* entchunk = find_chunk(gst, ent->position);
    if(entchunk->index != ent->chunk->index) {
        chunk_relocate_enemy(ent, entchunk);
    }
}

void render_enemy(struct state_t* gst, struct enemy_t* ent) {

    if(!ent->alive) {
        return;
    }

    if(!ent->enabled) {
        return;
    }
    if(ent->dist_to_player > gst->render_dist) {
        return;
    }

    int skip_view_test = (ent->dist_to_player < 200);
    if(!skip_view_test && !point_in_player_view(gst, &gst->player, ent->position, 90.0)) {
        return;
    }

    if(ent->render_callback) {
        ent->render_callback(gst, ent);
    }
}

void enemy_damage(
        struct state_t* gst,
        struct enemy_t* ent,
        float damage,
        struct hitbox_t* hitbox,
        Vector3 hit_position,
        Vector3 hit_direction,
        float knockback
){

    hit_direction = Vector3Normalize(hit_direction);

    ent->health -= damage * hitbox->damage_mult;
    hitbox->hits++;

    add_particles(gst,
            &gst->psystems[ENEMY_HIT_PSYS],
            GetRandomValue(30, 50),
            hit_position,
            hit_direction,
            (Color){ 255, 120, 0, 255 },
            NULL, NO_EXTRADATA, NO_IDB
            );

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
        ent->hit_callback(gst, ent, hit_position, hit_direction, knockback);
    }

    ent->despawn_timer = 0.0;
    ent->was_hit = 1;
    printf("HITBOX:%i, hits:%i\n", hitbox->tag, hitbox->hits);
    //printf("'%s': %0.2f | Health: %0.2f\n", __func__, damage, ent->health);
}

void enemy_death(struct state_t* gst, struct enemy_t* ent) {

    create_explosion(gst, ent->position, 125/*damage*/, 200.0/*radius*/);
    int xp_gain_bonus = 0;

    for(size_t i = 0; i < ent->num_hitboxes; i++) {
        if(ent->hitboxes[i].tag != HITBOX_HEAD) {
            continue;
        }

        xp_gain_bonus += ent->hitboxes[i].hits;
    }

    gst->player.kills[ent->type]++;

    xp_gain_bonus = CLAMP(xp_gain_bonus, 0, ENEMY_XP_GAIN_MAX_BONUS);
    printf("xp-gain: %i\n", ent->xp_gain + xp_gain_bonus);
    
    player_add_xp(gst, ent->xp_gain + xp_gain_bonus);

    if(ent->death_callback) {
        ent->death_callback(gst, ent);
    }

    struct ent_spawnsys_t* spawnsys = &gst->enemy_spawn_systems[ent->type];
    spawnsys->spawn_timer += spawnsys->add_time_when_killed;

    if(spawnsys->to_nextlevel_kills <= gst->player.kills[ent->type]) {
        increase_spawnsys_difficulty(gst, spawnsys);
    }

    if((gst->player.kills[ent->type] >= spawnsys->kills_to_next_spawnsys)
    && (spawnsys->next_spawnsys > 0)) {
        printf("\033[36m(SpawnSystem): EnemyLVL%i is able to spawn now!\033[0m\n", spawnsys->next_spawnsys);
        gst->enemy_spawn_systems[spawnsys->next_spawnsys].can_spawn = 1;
        spawnsys->next_spawnsys = -1;
    }
    
    chunk_remove_enemy(ent);
}

void enemy_add_hitbox(
        struct enemy_t* ent, 
        Vector3 hitbox_size,
        Vector3 hitbox_offset,
        float damage_multiplier,
        int hitbox_tag
){
    if(ent->num_hitboxes+1 >= MAX_HITBOXES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Trying to add too many hitboxes\033[0m\n",
                __func__);
        return;
    }

    ent->hitboxes[ent->num_hitboxes] = (struct hitbox_t) {
        .size = hitbox_size,
        .offset = hitbox_offset,
        .damage_mult = damage_multiplier,
        .tag = hitbox_tag,
        .hits = 0
    };

    ent->num_hitboxes++;
}

int create_enemy(
        struct state_t* gst,
        struct enemy_t* entptr,
        int enemy_type,
        int mood,
        Vector3 position
){

    switch(enemy_type) {
    
        case ENEMY_LVL0:
            {
                if(position.y <= gst->terrain.water_ylevel) {
                    fprintf(stderr, "\033[31m(ERROR) '%s': Enemy type '%i' cannot spawn in water.\033[0m\n",
                            __func__, enemy_type);
                    return 0;
                }

                if(!create_enemy_ext(gst,
                        entptr,
                        enemy_type,
                        mood,
                        &gst->enemy_models[enemy_type],
                        &gst->psystems[ENEMY_WEAPON_PSYS],
                        &gst->enemy_weapons[enemy_type],
                        ENEMY_LVL0_MAX_HEALTH,
                        position,
                        35,    /* XP Gain */
                        780.0, /* Target Range */
                        0,     /* Fov is ignored because its the full range. */
                        0.2,   /* Firerate */
                        enemy_lvl0_update,
                        enemy_lvl0_render,
                        enemy_lvl0_death,
                        enemy_lvl0_created,
                        enemy_lvl0_hit
                )) {
                    return 0;
                }

                // Head hitbox.
                enemy_add_hitbox(entptr,
                        (Vector3){ 33.0, 18.0, 33.0 },
                        (Vector3){ 0.0, 26.0, 0.0 },
                        1.758, // Damage multiplier.
                        HITBOX_HEAD
                        );

                // Legs hitbox.
                enemy_add_hitbox(entptr,
                        (Vector3){ 35.0, 12.0, 35.0 }, // Size
                        (Vector3){ 0.0, 5.0, 0.0 },    // Offset
                        0.25, // Damage multiplier.
                        HITBOX_LEGS
                        );

                entptr->spawn_callback(gst, entptr);
            }
            break;

        case ENEMY_LVL1:
            {
                if(position.y <= gst->terrain.water_ylevel) {
                    fprintf(stderr, "\033[31m(ERROR) '%s': Enemy type '%i' cannot spawn in water.\033[0m\n",
                            __func__, enemy_type);
                    return 0;
                }

                if(!create_enemy_ext(gst,
                        entptr,
                        enemy_type,
                        mood,
                        &gst->enemy_models[enemy_type],
                        &gst->psystems[ENEMY_WEAPON_PSYS],
                        &gst->enemy_weapons[enemy_type],
                        ENEMY_LVL1_MAX_HEALTH,
                        position,
                        60,    /* XP Gain */
                        680.0, /* Target Range */
                        90.0,  /* Target FOV */
                        0.3,   /* Firerate */
                        enemy_lvl1_update,
                        enemy_lvl1_render,
                        enemy_lvl1_death,
                        enemy_lvl1_created,
                        enemy_lvl1_hit
                )) { 
                    return 0;
                }

                // Head hitbox.
                enemy_add_hitbox(entptr,
                        (Vector3){ 13.0, 13.0, 13.0 }, // Size
                        (Vector3){ 0.0, 1.0, 0.0 },    // Offset
                        1.758, // Damage multiplier.
                        HITBOX_HEAD
                        );

                entptr->spawn_callback(gst, entptr);
            }
            break;


        default:
            fprintf(stderr, "\033[31m(ERROR) '%s': Invalid enemy type '%i'\033[0m\n",
                    __func__, enemy_type);
            break;
    }

    return 1;
}


int enemy_can_see_player(struct state_t* gst, struct enemy_t* ent) {
    return (!_is_terrain_blocking_view(gst, ent) && (ent->dist_to_player <= ent->target_range));
}

int player_in_enemy_fov(struct state_t* gst, struct enemy_t* ent, Matrix* body_matrix) {

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
    float fov = map(dot, -1.0, 1.0, 0.0, 180.0);

    return (fov < ent->target_fov);
}

int enemy_has_target(
        struct state_t* gst, struct enemy_t* ent, Matrix* ent_body_matrix,
        void(*target_found) (struct state_t*, struct enemy_t*),
        void(*target_lost)   (struct state_t*, struct enemy_t*)
){
    int in_fov = player_in_enemy_fov(gst, ent, ent_body_matrix) || ent->was_hit;
    int has_target_now = in_fov && enemy_can_see_player(gst, ent);

    if((ent->mood == ENT_HOSTILE) && has_target_now && !ent->has_target) {
        if(ent->time_from_target_lost > 0.5) {
            ent->has_target = 1;
            ent->time_from_target_found = 0.0;
            target_found(gst, ent);
        }
    }
    else
    if(((!has_target_now || !in_fov) && ent->has_target)) {
        if(ent->time_from_target_found > 0.5) {
            ent->has_target = 0;
            ent->time_from_target_lost = 0.0;
            target_lost(gst, ent);
        }
    }

    if(ent->has_target) {
        ent->time_from_target_found += gst->dt;
    }
    else {
        ent->time_from_target_lost += gst->dt;
    }

    return ent->has_target;
}

int num_enemies_in_radius(struct state_t* gst, int enemy_type, float radius, int* num_in_world) {
    int num = 0;

    printf("%s: Not reimplemented.\n", __func__);
    /*
    for(size_t i = 0; i < gst->num_enemies; i++) {
        if(!gst->enemies[i].alive || !gst->enemies[i].enabled) {
            continue;
        }
        if(gst->enemies[i].type != enemy_type) {
            continue;
        }

        if(gst->enemies[i].dist_to_player < radius) {
            num++;
        }

        if(num_in_world) {
            *num_in_world += 1;
        }
    }
    */

    return num;
}

static Vector3 get_good_spawn_pos(struct state_t* gst, float spawn_radius) {
    Vector3 pos = (Vector3) { 0, 0, 0 };
    const int max_attemps = 300;
    int attemps = 0;

    while(attemps < max_attemps) {
        pos = (Vector3) {
            gst->player.position.x + RSEEDRANDOMF(-spawn_radius, spawn_radius),
                0,
            gst->player.position.z + RSEEDRANDOMF(-spawn_radius, spawn_radius)
        };

        float dist = Vector3Distance(pos, (Vector3){ gst->player.position.x, 0, gst->player.position.z });

        if(dist > ENEMY_SPAWN_SAFE_RADIUS) {
            break;
        }

        attemps++;
    }

    if(attemps >= max_attemps) {
        printf("\033[35m(WARNING) '%s': Failed to get good spawn position for enemy\033[0m\n",
                __func__);
        pos.x = gst->player.position.x + ENEMY_SPAWN_SAFE_RADIUS*2;
        pos.z = gst->player.position.z + ENEMY_SPAWN_SAFE_RADIUS*2;
    }

    return pos;
}


void update_enemy_spawn_systems(struct state_t* gst) {

    printf("%s not implemented yet.\n", __func__);

}

void setup_default_enemy_spawn_settings(struct state_t* gst) {


    // --- ENEMY_LVL0 Defaults ---
    {
        struct ent_spawnsys_t* spawnsys = &gst->enemy_spawn_systems[ENEMY_LVL0];

        spawnsys->enemy_type = ENEMY_LVL0;
        spawnsys->max_difficulty = ENEMY_LVL0_MAX_DIFFICULTY;
        spawnsys->can_spawn = 1;
        spawnsys->difficulty = 0;
        spawnsys->max_in_world = 10;
        spawnsys->max_in_spawn_radius = 6;
        spawnsys->spawn_radius = 1000.0;
        spawnsys->spawn_delay = 30.0;
        spawnsys->num_spawns_min = 3;
        spawnsys->num_spawns_max = 4;
        spawnsys->nextlevel_kills_add = 10;
        spawnsys->to_nextlevel_kills = 10;
        spawnsys->add_time_when_killed = 5.0;

        spawnsys->next_spawnsys = ENEMY_LVL1;
        spawnsys->kills_to_next_spawnsys = 8;
        
        // Skip little bit ahead to not let the player keep waiting for too long.
        spawnsys->spawn_timer = spawnsys->spawn_delay-2.0;
    }

    // --- ENEMY_LVL1 Defaults ---
    {
        struct ent_spawnsys_t* spawnsys = &gst->enemy_spawn_systems[ENEMY_LVL1];

        spawnsys->enemy_type = ENEMY_LVL1;
        spawnsys->max_difficulty = ENEMY_LVL1_MAX_DIFFICULTY;
        spawnsys->can_spawn = 0;
        spawnsys->difficulty = 0;
        spawnsys->max_in_world = 10;
        spawnsys->max_in_spawn_radius = 6;
        spawnsys->spawn_radius = 1000.0;
        spawnsys->spawn_delay = 40.0;
        spawnsys->num_spawns_min = 1;
        spawnsys->num_spawns_max = 2;
        spawnsys->nextlevel_kills_add = 10;
        spawnsys->to_nextlevel_kills = 20;
        spawnsys->add_time_when_killed = 2.0;
        spawnsys->spawn_timer = spawnsys->spawn_delay-3.0;
        
        spawnsys->next_spawnsys = -1;
        spawnsys->kills_to_next_spawnsys = 0;
    }
}

void increase_spawnsys_difficulty(struct state_t* gst, struct ent_spawnsys_t* spawnsys) {

    if(spawnsys->difficulty >= spawnsys->max_difficulty) {
        return;
    }
    
    spawnsys->to_nextlevel_kills += spawnsys->nextlevel_kills_add;
    spawnsys->num_spawns_max += 2;

    // Increase weapon accuracy and projectile speed.

    struct weapon_t* ent_weapon = &gst->enemy_weapons[spawnsys->enemy_type];
    ent_weapon->accuracy += 0.15;
    ent_weapon->prj_speed += 85;


    printf("\033[36m(SpawnSystem): Increased EnemyLVL%i Difficulty\033[0m\n",
            spawnsys->enemy_type);
}


