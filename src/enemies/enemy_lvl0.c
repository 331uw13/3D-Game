
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

    *part->transform = position_m;
    part->alive = 1;
    part->lifetime = 0.0;
    part->max_lifetime = weapon->prj_max_lifetime;
}



static Vector3 _normalize(Vector3 a) {
    float len = Vector3Length(a);
    Vector3 d = (Vector3){len, len, len};
    return Vector3Divide(a, d);
}


void enemy_lvl0_update(struct state_t* gst, struct entity_t* ent, int render_setting) {
    const float dst2player = Vector3Distance(gst->player.position, ent->position);


    if(dst2player >= 300) {
        return;
    }



    // Rotate to terrain surface

    float y = 0.0;
    Matrix rotate_m = get_rotation_to_surface(&gst->terrain, ent->position.x, ent->position.z, &y);
    Matrix pos_m = MatrixTranslate(ent->position.x, y, ent->position.z);

    ent->model.transform = MatrixMultiply(rotate_m, pos_m);

    ent->position.y = y;


    int has_target_now = (dst2player <= ent->target_range);


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
                ent->angle_change += gst->dt * 4.0;

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

                // offset model.
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
                    weapon_add_projectile(gst, ent->weapon, prj_position, prj_direction);
                    ent->firerate_timer = 0.0;
                
                    ent->gun_index = !ent->gun_index;
                }

            }
            break;

    }



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


void enemy_lvl0_hit(struct state_t* gst, struct entity_t* ent) {
}

void enemy_lvl0_death(struct state_t* gst, struct entity_t* ent) {
}

void enemy_lvl0_created(struct state_t* gst, struct entity_t* ent) {
    ent->state = ENT_STATE_SEARCHING_TARGET;



}


