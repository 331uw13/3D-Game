#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "state.h"
#include "util.h"


void free_enemyarray(struct state_t* gst) {
    if(gst->enemies) {   
        for(size_t i = 0; i < gst->num_enemies; i++) {
            
            delete_enemy(&gst->enemies[i]);
            /*
            if(gst->enemies[i].model_loaded) {
                UnloadModel(gst->enemies[i].model);
                gst->enemies[i].model_loaded = 0;
            }
            */
        }

        free(gst->enemies);
    }

    printf("\033[32m >> Deleted Enemies.\033[0m\n");
}


struct enemy_t* create_enemy(
        struct state_t* gst, 
        const char* model_filepath,
        int texture_id,
        int max_health,
        Vector3 hitbox,
        Vector3 position
        )
{

    struct enemy_t* ptr = NULL;


    long int new_num_elem = 0;
    gst->enemies = m_resize_array(
            gst->enemies,
            sizeof *gst->enemies,
            gst->enemyarray_size,
            gst->enemyarray_size + 1,
            &new_num_elem
            );

    if(new_num_elem == MEMRESIZE_ERROR) {
        goto error;
    }

    long int id = gst->num_enemies;
    gst->num_enemies++;

    ptr = &gst->enemies[id];
    ptr->id = id;
    ptr->health = max_health;
    ptr->max_health = max_health;
    ptr->position = position;
    ptr->hitbox = hitbox;
    ptr->dest_reached = 1;
    ptr->travel_dest = position;
    ptr->travel_start = position;
    ptr->travelled = 0.0;
    ptr->state = ENEMY_SEARCH;
    ptr->knockback_velocity = (Vector3) { 0.0, 0.0, 0.0 };
    ptr->hit_direction = (Vector3) { 0.0, 0.0, 0.0 };
    ptr->rotation_from_hit = (Vector3) { 0.0, 0.0, 0.0 };
    ptr->was_hit = 0;
    ptr->forward_angle = 0.0;
    ptr->angle_change = 0.0;
    ptr->previous_angle = 0.0;


    ptr->model_loaded = setup_3Dmodel(gst, &ptr->model, model_filepath, texture_id, position);
    if(!ptr->model_loaded) {
        goto error;
    }



    gst->enemyarray_size = new_num_elem;


    printf("\033[34m >> Created Enemy. ID(%li), Health(%i), Model '%s'\033[35m enemyarray_size: %li\033[0m\n", 
            id, max_health, model_filepath, gst->enemyarray_size);


    ptr->torus = GenMeshTorus(0.2, 3.0, 8, 8);
    ptr->torus_material = LoadMaterialDefault();
    ptr->torus_material.shader = gst->shaders[ENEMY_HOVER_EFFECT_SHADER];
    
error:
    return ptr;
}

void delete_enemy(struct enemy_t* enemy) {
    if(IsModelValid(enemy->model)) {
        UnloadModel(enemy->model);
        enemy->model_loaded = 0;
    }

    UnloadMesh(enemy->torus);

}


void move_enemy(struct enemy_t* enemy, Vector3 position) {
    if(enemy->model_loaded) { 
        enemy->position = position;
        matrix_addtransl(&enemy->model.transform, position.x, position.y, position.z);
    }
}

void enemy_pick_random_destination(struct enemy_t* enemy) {
    
    
    int valid_pos = 0;
    Vector3 p = (Vector3) { 0 };

    while(!valid_pos) {
    
        p = (Vector3){
                GetRandomValue(
                        enemy->position.x - ENEMY_RND_SEARCH_RADIUS,
                        enemy->position.x + ENEMY_RND_SEARCH_RADIUS
                        ),
                    
                enemy->position.y,

                GetRandomValue(
                        enemy->position.z - ENEMY_RND_SEARCH_RADIUS,
                        enemy->position.z + ENEMY_RND_SEARCH_RADIUS
                        )
        };

        if(Vector3Distance(p, enemy->position) >= ENEMY_RND_SEARCH_MIN_RADIUS) {
            valid_pos = 1;
        }
    }
    
    enemy->travel_dest = p;


}

void render_enemy(struct state_t* gst, struct enemy_t* enemy) {
    if(enemy->health > 0) {
        
        DrawMesh(
                enemy->model.meshes[0],
                enemy->model.materials[0],
                enemy->model.transform
                );



        // draw hovering effect circles
        
        //BeginShaderMode(gst->shaders[ENEMY_HOVER_EFFECT_SHADER]);
        
        const float time = GetTime()*8 + enemy->id;
        
        Vector3 center = enemy->position;
        center.y -= 1.2;


        Color color = (Color) { 25, 255, 10, 150 };

        //DrawCircle3D(center, sin(time)*0.2+0.8, (Vector3){1, 0, 0}, 90, color);


        Matrix transform = MatrixTranslate(center.x, center.y, center.z);
        Matrix rotation = MatrixRotateXYZ((Vector3){ 90*DEG2RAD, 0.0, 0.0 });

        float sc = sin(time)*0.2+0.8;
        Matrix scale = MatrixScale(sc, sc, sc);

        transform = MatrixMultiply(rotation, transform);
        transform = MatrixMultiply(scale, transform);


        DrawMesh(enemy->torus, enemy->torus_material, transform);


        //BeginShaderMode(gst->shaders[DEFAULT_SHADER]);

    }

    
}

void update_enemy(struct state_t* gst, struct enemy_t* enemy) {

    if(!enemy->model_loaded) {
        return;
    }

    
    Vector3 pos = enemy->position;

    float heightvalue = get_smooth_heightmap_value(&gst->terrain, pos.x, pos.z);

    pos.y = 
        sin(enemy->id*3 + GetTime()*3.0) * 0.3 
        + heightvalue + 3;

    enemy->travel_dest.y = pos.y;

    move_enemy(enemy, pos);
    
    enemy->was_hit = 
        !FloatEquals(enemy->knockback_velocity.x, 0.0) &&
        !FloatEquals(enemy->knockback_velocity.y, 0.0) &&
        !FloatEquals(enemy->knockback_velocity.z, 0.0)
        ;


    if(enemy->was_hit) {
        const float hit_friction = 0.9;
        pos = Vector3Add(pos, enemy->knockback_velocity);
        enemy->knockback_velocity.x *= hit_friction;
        enemy->knockback_velocity.y *= hit_friction;
        enemy->knockback_velocity.z *= hit_friction;

        float rf = 0.94;
        enemy->rotation_from_hit.x *= rf;
        enemy->rotation_from_hit.z *= rf;
        enemy->rotation_from_hit.y *= rf;

        enemy->model.transform = MatrixRotateXYZ((Vector3){
                enemy->rotation_from_hit.z,
                enemy->rotation_from_hit.x + enemy->forward_angle, 
                enemy->rotation_from_hit.y 
                });
        
        enemy->dest_reached = 1;
        move_enemy(enemy, pos);
    
    }
    else if(enemy->state == ENEMY_TURNING) {

        float angle = lerp(enemy->angle_change, enemy->previous_angle, enemy->forward_angle);
        enemy->model.transform = MatrixRotateXYZ((Vector3){ 0.0, angle, 0.0 });
        enemy->angle_change += gst->dt * 2.0;

        if(enemy->angle_change >= 1.0) {
            enemy->state = ENEMY_SEARCH;
        }

        move_enemy(enemy, pos);
    }
    else if(enemy->state == ENEMY_SEARCH) {

        if(enemy->dest_reached) {
            enemy->travelled = 0.0;
            enemy_pick_random_destination(enemy);

            enemy->travel_start = enemy->position;
            enemy->dest_reached = 0;

            enemy->state = ENEMY_TURNING;
            
            enemy->angle_change = 0.0;
            enemy->previous_angle = enemy->forward_angle;
            enemy->forward_angle = angle_xz(pos, enemy->travel_dest);
            
            return;
        }
        


        // smooth the start and end.
        // -------- TODO: make this better:
        float t = 0.5+0.5*sin(enemy->travelled - (M_PI/2.0));

        pos.x = lerp(t, enemy->travel_start.x, enemy->travel_dest.x);
        pos.z = lerp(t, enemy->travel_start.z, enemy->travel_dest.z);


        float dist = Vector3Distance(pos, enemy->travel_dest);
        
        // try to normalize the speed.
        // otherwise smaller distances the enemy moves slower.
        float n = Vector3Distance(enemy->travel_start, enemy->travel_dest) / ENEMY_RND_SEARCH_RADIUS;
        n = 1.0 / n;


        enemy->travelled += gst->dt * n;
        

        if(dist <= 3.0) {
            enemy->dest_reached = 1;
        }

        move_enemy(enemy, pos);
        return;
    }
}

void enemy_hit(struct state_t* gst, struct enemy_t* enemy, struct projectile_t* proj) {
    
    enemy->health -= gst->player.gun.proj_damage;
    enemy->knockback_velocity = Vector3Scale(proj->direction, gst->player.gun.knockback);
    enemy->hit_direction = proj->direction;

    float random_r_x = (float)GetRandomValue(-5.0, 5.0) / 5.0;
    float random_r_y = (float)GetRandomValue(-5.0, 5.0) / 20.0;
    float random_r_z = (float)GetRandomValue(-5.0, 5.0) / 8.0;
    
    enemy->rotation_from_hit = (Vector3){ random_r_x, random_r_y, random_r_z };

    struct enemyhit_psys_extra_t extradata = (struct enemyhit_psys_extra_t) {
        /* spawn position */       proj->position,
        /* projectile direction */ proj->direction
    };

    int num_particles = GetRandomValue(8, 20);
    add_particles(gst, &gst->psystems[PSYS_ENEMYHIT], num_particles, &extradata, HAS_EXTRADATA);
}


void enemy_hit_psys_pupdate(struct state_t* gst, struct psystem_t* psys, struct particle_t* p) {

    //p->velocity = Vector3Add(p->velocity, p->accel);
    p->position = Vector3Add(p->position, p->velocity);

    float n = normalize(p->lifetime, 0.0, p->max_lifetime);
    n *= n;
    float scale = lerp(n, p->initial_scale, 0.0);
  
    p->velocity.y += p->accel.y;

    Matrix position_m = MatrixTranslate(p->position.x, p->position.y, p->position.z);
    Matrix scale_m = MatrixScale(scale, scale, scale);

    position_m = MatrixMultiply(scale_m, position_m);

    *p->transform = position_m;//MatrixTranslate(p->position.x, p->position.y, p->position.z);
}


void enemy_hit_psys_pinit(struct state_t* gst, struct psystem_t* psys, struct particle_t* p, 
        void* extradata_ptr, int has_extradata) {
   
    if(!has_extradata) {
        return;
    }


    p->alive = 1;
    p->max_lifetime = 0.5;
    p->lifetime = 0.0;
    p->initial_scale = 1.3;
    p->scale = p->initial_scale;

    struct enemyhit_psys_extra_t* extradata = (struct enemyhit_psys_extra_t*)extradata_ptr; 
    p->position = extradata->spawn_position;

    const float rrad = 0.3;
    p->position.x += RSEEDRANDOMF(-rrad, rrad);
    p->position.y += RSEEDRANDOMF(-rrad, rrad);
    p->position.z += RSEEDRANDOMF(-rrad, rrad);


    p->velocity = Vector3Negate(extradata->proj_direction);
    p->velocity = vec3mult_v(p->velocity, 2.0); 
    p->velocity.x += RSEEDRANDOMF(-2.0, 2.0);
    p->velocity.y += RSEEDRANDOMF(-1.0, 2.8);
    p->velocity.z += RSEEDRANDOMF(-2.0, 2.0);

    p->accel.y = -0.1 * gst->dt;

    // TODO: fix bugs in particle system.
    // particle gets updated 2 times.
    // some kind of weird pause while updating when 'nextpart_index' is going back to 0

    p->velocity = vec3mult_v(p->velocity, gst->dt*3.0);

    *p->transform = MatrixTranslate(p->position.x, p->position.y, p->position.z);
}



