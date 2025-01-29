#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "state.h"
#include "util.h"


void free_enemyarray(struct state_t* gst) {
    if(gst->enemies) {   
        for(size_t i = 0; i < gst->num_enemies; i++) {
            
            if(gst->enemies[i].model_loaded) {
                UnloadModel(gst->enemies[i].model);
                gst->enemies[i].model_loaded = 0;
            }
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
    ptr->position = position;
    ptr->hitbox = hitbox;
    ptr->dest_reached = 1;
    ptr->travel_dest = position;
    ptr->travel_start = position;
    ptr->travelled = 0.0;
    ptr->state = ENEMY_SEARCH;

    ptr->model_loaded = setup_3Dmodel(gst, &ptr->model, model_filepath, position);
    if(!ptr->model_loaded) {
        goto error;
    }
   
    if(texture_id >= 0) {
        ptr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[texture_id];
    }

    gst->enemyarray_size = new_num_elem;


    printf("\033[34m >> Created Enemy. ID(%li), Health(%i), Model '%s'\033[35m enemyarray_size: %li\033[0m\n", 
            id, max_health, model_filepath, gst->enemyarray_size);


error:
    return ptr;
}

void move_enemy(struct enemy_t* enemy, Vector3 position) {
    if(enemy->model_loaded) { 
        enemy->position = position;
        matrix_addtransl(&enemy->model.transform, position.x, position.y, position.z);
    }
}


void update_enemy(struct state_t* gst, struct enemy_t* enemy) {

    if(!enemy->model_loaded) {
        return;
    }

    if(enemy->state == ENEMY_SEARCH) {
        
        Vector3 pos = enemy->position;

        if(enemy->dest_reached) {
            enemy->travelled = 0.0;
            enemy->travel_dest.x 
                = GetRandomValue(
                        pos.x - ENEMY_RANDOM_SEARCH_RADIUS,
                        pos.x + ENEMY_RANDOM_SEARCH_RADIUS
                        );

            enemy->travel_dest.z
                = GetRandomValue(
                        pos.z - ENEMY_RANDOM_SEARCH_RADIUS,
                        pos.z + ENEMY_RANDOM_SEARCH_RADIUS
                        );
            enemy->travel_start = enemy->position;
            enemy->dest_reached = 0;



            Vector3 diff = Vector3Subtract(pos, enemy->travel_dest);
            float angle = -(atan2(diff.z, diff.x) + M_PI);

            enemy->model.transform = MatrixRotateXYZ((Vector3){ 0.0, angle, 0.0 });

        }
 

        // smooth and normalize the start and end.
        float t = 0.5+0.5*sin(enemy->travelled - (M_PI/2.0));

        pos.x = lerp(t, enemy->travel_start.x, enemy->travel_dest.x);
        pos.z = lerp(t, enemy->travel_start.z, enemy->travel_dest.z);


        float dist = Vector3Distance(pos, enemy->travel_dest);
        
        // try to normalize the speed.
        // otherwise smaller distances the enemy moves slower.
        float n = Vector3Distance(enemy->travel_start, enemy->travel_dest) / ENEMY_RANDOM_SEARCH_RADIUS;
        n = 1.0 / n;


        enemy->travelled += gst->dt * n;
        

        if(dist <= 3.0) {
            enemy->dest_reached = 1;
        }





        move_enemy(enemy, pos);
    

        return;
    }





}










