#include <math.h>
#include <stdio.h>

#include "state/state.h"
#include "npc.h"
#include "util.h"

#include <raymath.h>



void setup_npc(struct state_t* gst, struct npc_t* npc) {
    npc->position = (Vector3){ 0, 0, 0 };
    
    npc->model = LoadModel("res/models/npc.glb");
    npc->model.materials[0] = LoadMaterialDefault();
    npc->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[MUSHROOM_BODY_TEXID];

    npc->model.materials[1] = LoadMaterialDefault();
    npc->model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[MUSHROOM_HAT_TEXID];

    npc->prev_position = (Vector3){0};
    npc->rotation = (Vector3){0};
    npc->dist2player = 0.0;
    npc->active = 0;

    npc->Q_target = QuaternionIdentity();
    npc->Q_prev = QuaternionIdentity();
    npc->Q_now = QuaternionIdentity();
    npc->angle_change = 1.0;

    npc->status = NPC_STATUS_IDLE;
    npc->leg_matrix[NPC_L_LEG_MI] = MatrixIdentity();
    npc->leg_matrix[NPC_R_LEG_MI] = MatrixIdentity();

    npc->travel.dest_reached = 1;

    gst->init_flags |= INITFLG_NPC;
}

void delete_npc(struct state_t* gst, struct npc_t* npc) {
    if(!(gst->init_flags & INITFLG_NPC)) {
        return;
    }
    UnloadModel(npc->model);

    printf("\033[35m -> Deleted NPC.\033[0m\n");
}


static void get_npc_travel_dest(struct state_t* gst, struct npc_t* npc) {
    
    npc->travel.start = (Vector3){ npc->position.x, 0, npc->position.z };
    npc->travel.travelled = 0.0;
    npc->travel.dest_reached = 0;

    Vector3 dst_pos = (Vector3) {
        npc->position.x + RSEEDRANDOMF(-NPC_RND_DEST_RADIUS, NPC_RND_DEST_RADIUS),
        0, // y can be ignored.
        npc->position.z + RSEEDRANDOMF(-NPC_RND_DEST_RADIUS, NPC_RND_DEST_RADIUS)
    };

    npc->travel.dest = dst_pos;
}


void update_npc(struct state_t* gst, struct npc_t* npc) {
    if(!npc->active) {
        return;
    }

    npc->dist2player = Vector3Distance(gst->player.position, npc->position);

    RayCollision ray;    

    // Rotate to terrain surface.
    Matrix rotation = get_rotation_to_surface(&gst->terrain, npc->position.x, npc->position.z, &ray);
    npc->position.y = ray.point.y;
    npc->model.transform = MatrixTranslate(npc->position.x, npc->position.y, npc->position.z);
    npc->model.transform = MatrixMultiply(rotation, npc->model.transform);

    int in_talk_dist = (npc->dist2player <= NPC_TALK_DISTANCE);

    if(in_talk_dist && (npc->status != NPC_STATUS_PLAYER_NEARBY)) {
        npc->status = NPC_STATUS_PLAYER_NEARBY;

        npc->angle_change = 0.0;
        npc->Q_prev = npc->Q_now;
    }
    else
    if(!in_talk_dist && (npc->status == NPC_STATUS_PLAYER_NEARBY)) {
        npc->status = NPC_STATUS_WALKING;
        npc->angle_change = 0.0;
        npc->Q_prev = npc->Q_now;
    }


    if(npc->angle_change < 1.0) {
        npc->angle_change += gst->dt;
    }

    npc->model.transform = MatrixMultiply(
            QuaternionToMatrix(npc->Q_now),
            npc->model.transform
            );


    npc->leg_matrix[NPC_L_LEG_MI] = npc->model.transform;
    npc->leg_matrix[NPC_R_LEG_MI] = npc->model.transform;

    switch(npc->status) {
        case NPC_STATUS_WALKING:
            {

                float anim_tm = inout_cubic(npc->travel.travelled) * 5.0+5.0;

                // Move legs.
                float L_leg_y = sin(gst->time*anim_tm)*0.5+0.5;
                float R_leg_y = sin(gst->time*anim_tm+M_PI)*0.5+0.5; // Phase shift right leg.

                // TODO: Use terrain normal for these translations.
                npc->leg_matrix[NPC_L_LEG_MI] 
                    = MatrixMultiply(npc->leg_matrix[NPC_L_LEG_MI],MatrixTranslate(0.0, L_leg_y, 0.0));
                
                npc->leg_matrix[NPC_R_LEG_MI] 
                    = MatrixMultiply(npc->leg_matrix[NPC_R_LEG_MI],MatrixTranslate(0.0, R_leg_y, 0.0));


                if(npc->travel.dest_reached) {
                    get_npc_travel_dest(gst, npc);
                }

                npc->travel.travelled += gst->dt*0.2;

                float t = inout_cubic(npc->travel.travelled);
                npc->position = Vector3Lerp(npc->travel.start, npc->travel.dest, t);
                npc->position.y = ray.point.y;

                if(Vector3Distance((Vector3){ npc->position.x, 0, npc->position.z},
                            npc->travel.dest) < 3.0) {
                    npc->travel.dest_reached = 1;
                    npc->Q_prev = npc->Q_now;
                    npc->angle_change = 0.0;
                }

                if(npc->angle_change < 1.0) {
                    npc->Q_target = QuaternionFromMatrix(MatrixRotateY(angle_xz(npc->travel.dest, npc->position)));
                    npc->Q_now = QuaternionSlerp(npc->Q_prev, npc->Q_target, inout_cubic(npc->angle_change));
                }
            }
            break;

        case NPC_STATUS_PLAYER_NEARBY: 
            {
                // First need to rotate towards player
                if(npc->angle_change < 1.0) {
                    // Also update the target if the player has moved after entering "talk distance"
                    npc->Q_target = QuaternionFromMatrix(MatrixRotateY(angle_xz(gst->player.position, npc->position)));
                    npc->Q_now = QuaternionSlerp(npc->Q_prev, npc->Q_target, inout_cubic(npc->angle_change));
                    return;
                }

                npc->Q_now = QuaternionFromMatrix(MatrixRotateY(angle_xz(gst->player.position, npc->position)));

            }
            break;
    }

    npc->prev_position = npc->position;

}


void render_npc(struct state_t* gst, struct npc_t* npc) {
    if(!npc->active) {
        return;
    }

    // Body
    DrawMesh(
            npc->model.meshes[0],
            npc->model.materials[0],
            npc->model.transform
            );

    // Hat
    DrawMesh(
            npc->model.meshes[1],
            npc->model.materials[1],
            npc->model.transform
            );

    // Right leg
    DrawMesh(
            npc->model.meshes[2],
            npc->model.materials[0],
            npc->leg_matrix[NPC_R_LEG_MI]
            );

    // Left leg
    DrawMesh(
            npc->model.meshes[3],
            npc->model.materials[0],
            npc->leg_matrix[NPC_L_LEG_MI]
            );


}

