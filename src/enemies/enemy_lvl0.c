
#include "enemy_lvl0.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PROJECTILE PARTICLE UPDATE ---
void enemy_lvl0_weapon_psystem_projectile_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    struct weapon_t* weapon = (struct weapon_t*)psys->userptr;
    if(!weapon) {
        MISSING_PSYSUSERPTR;
        return;
    }


    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = position_m;
    
    part->light.position = part->position;
    update_light_values(&part->light, gst->shaders[DEFAULT_SHADER]);



    // Check collision with terrain.

    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);

    if(t_hit.point.y >= part->position.y) {
        add_particles(gst, 
                &gst->psystems[PROJECTILE_ELVL0_ENVHIT_PSYSTEM], 1,
                part->position, part->velocity, NULL, NO_EXTRADATA);
       
        disable_particle(gst, part);
        return;
    }


}

// PROJECTILE PARTICLE INITIALIZATION ---
void enemy_lvl0_weapon_psystem_projectile_pinit(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    struct weapon_t* weapon = (struct weapon_t*)psys->userptr;
    if(!weapon) {
        MISSING_PSYSUSERPTR;
        return;
    }


    part->velocity = velocity;
    part->position = origin;
    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    add_projectile_light(gst, &part->light, part->position, weapon->prj_color, gst->shaders[DEFAULT_SHADER]);
    part->has_light = 1;

    *part->transform = position_m;
    part->max_lifetime = weapon->prj_max_lifetime;
}



static int terrain_blocking_view(struct state_t* gst, struct entity_t* ent) {
    int result = 0;

    Vector3 ent_direction = Vector3Normalize(Vector3Subtract(gst->player.position, ent->position));
    Vector3 ray_position = ent->position;

    // Move 'ray_position' towards player
    // and cast ray from 'terrain.highest_point' at ray_position.X and Z.
    // to see if the hit Y position is bigger than ray Y position, if so terrain was hit.
    // this is not perfect but will do for now i guess.

    Vector3 step = Vector3Scale(ent_direction, 3.0);
    const int max_steps = 100;
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

void enemy_lvl0_update(struct state_t* gst, struct entity_t* ent) {


    if(ent->health <= 0.001) {
        return;
    }

    // Rotate to terrain surface

    float y = 0.0;
    Matrix rotate_m = get_rotation_to_surface(&gst->terrain, ent->position.x, ent->position.z, &y);
    Matrix pos_m = MatrixTranslate(ent->position.x, y, ent->position.z);

    ent->model.transform = MatrixMultiply(rotate_m, pos_m);
    ent->position.y = y;


    float dst2player = Vector3Distance(ent->position, gst->player.position);
    int has_target_now 
        = (dst2player <= ent->target_range) 
        && !terrain_blocking_view(gst, ent);


    ent->body_matrix = ent->model.transform;

    if(has_target_now && !ent->has_target) {
        printf(" enemy -> Target Found!\n");
        ent->state = ENT_STATE_CHANGING_ANGLE;
                
        ent->angle_change = 0.0;
        Matrix prev_angle_m = MatrixRotateY(ent->forward_angle);

        ent->Q1 = QuaternionFromMatrix(prev_angle_m);
        ent->forward_angle = angle_xz(gst->player.position, ent->position);

        Matrix rm = MatrixRotateY(ent->forward_angle);
        ent->Q0 = QuaternionFromMatrix(rm);
    }
    else
    if(!has_target_now && ent->has_target) {
        printf(" enemy -> Target Lost.\n");
        ent->state = ENT_STATE_SEARCHING_TARGET;
    
    }
    ent->has_target = has_target_now;


    if(ent->firerate_timer < ent->firerate) {
        ent->firerate_timer += gst->dt;
    }


    switch(ent->state) {
       
        case ENT_STATE_WASHIT:
            {
                ent->stun_timer += gst->dt;

                Matrix rm = MatrixRotateXYZ((Vector3){
                        ent->rotation_from_hit.x,
                        ent->rotation_from_hit.y + ent->forward_angle,
                        ent->rotation_from_hit.z
                        });

                rm = MatrixMultiply(rm, ent->body_matrix);
                ent->body_matrix = rm;

                ent->position.x += ent->knockback_velocity.x * gst->dt;
                ent->position.z += ent->knockback_velocity.z * gst->dt;

                float restore_angle = 0.99;
                ent->rotation_from_hit.x *= restore_angle;
                ent->rotation_from_hit.y *= restore_angle;
                ent->rotation_from_hit.z *= restore_angle;

                ent->knockback_velocity.x *= 0.99;
                ent->knockback_velocity.z *= 0.99;


                if(ent->stun_timer >= ent->max_stun_time) {
                    ent->state = ENT_STATE_SEARCHING_TARGET;
                    ent->has_target = 0;
                }
            }
            break;
        case ENT_STATE_SEARCHING_TARGET:
            {
                ent->forward_angle += gst->dt;
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->body_matrix);
                ent->body_matrix = rm;
            }
            break;

        case ENT_STATE_CHANGING_ANGLE:
            {
                Quaternion Q = QuaternionSlerp(ent->Q1, ent->Q0, ent->angle_change);
                ent->angle_change += gst->dt * 5.0;

                if(ent->angle_change > 1.0) {
                    ent->state = ENT_STATE_HAS_TARGET;
                }

                Matrix qm = QuaternionToMatrix(Q);
                ent->body_matrix = MatrixMultiply(qm, ent->body_matrix);
      
            }
            break;

        case ENT_STATE_HAS_TARGET:
            {
                ent->forward_angle = angle_xz(gst->player.position, ent->position);
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->body_matrix);
                ent->body_matrix = rm;
    
                Vector3 prj_position = ent->position;


                float angle = ent->forward_angle;
                const float ang_rad = 1.5;


                if(!gst->player.noclip) {
                    prj_position.x += 0.2;
                    prj_position.z -= 0.4;

                    if(ent->gun_index) {
                        prj_position.x += ang_rad*sin(angle);
                        prj_position.z += ang_rad*cos(angle);
                    }
                    else {
                        prj_position.x -= ang_rad*sin(angle);
                        prj_position.z -= ang_rad*cos(angle);
                    }
                   
                    prj_position.y += 1.0;



                    Vector3 prj_direction = Vector3Normalize(Vector3Subtract(gst->player.position, prj_position));
                    
                    if(ent->firerate_timer >= ent->firerate) {
                        weapon_add_projectile(
                                gst,
                                ent->weapon,
                                prj_position,
                                prj_direction,
                                ent->weapon->accuracy
                                );
                        ent->firerate_timer = 0.0;
                    
                        ent->gun_index = !ent->gun_index;
                    }
                }

            }
            break;
    }

}

void enemy_lvl0_render(struct state_t* gst, struct entity_t* ent) {

    // Turret body

    DrawMesh(
            ent->model.meshes[0],
            ent->model.materials[0],
            ent->body_matrix
            );


    // Turret legs

    DrawMesh(
            ent->model.meshes[1],
            ent->model.materials[0],
            ent->model.transform
            );
}


void enemy_lvl0_hit(struct state_t* gst, struct entity_t* ent, float damage, 
        Vector3 hit_direction, Vector3 hit_position) {
   
    const float rd = 0.35;
    ent->rotation_from_hit = (Vector3) {
        RSEEDRANDOMF(-rd, rd),
        RSEEDRANDOMF(-rd, rd),
        RSEEDRANDOMF(-rd, rd)
    };

    ent->knockback_velocity = Vector3Scale(hit_direction, 15.0);

    printf("%f\n", ent->health);

    ent->max_stun_time = 0.5;
    ent->stun_timer = 0.0;
    
    ent->previous_state = ent->state;
    ent->state = ENT_STATE_WASHIT;
}

void enemy_lvl0_death(struct state_t* gst, struct entity_t* ent) {
    printf("Enemy %i Died\n", ent->index);
}

void enemy_lvl0_created(struct state_t* gst, struct entity_t* ent) {
    ent->state = ENT_STATE_SEARCHING_TARGET;

}


