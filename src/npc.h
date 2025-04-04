#ifndef GAME_NPC_H
#define GAME_NPC_H


#include <raylib.h>


// The NPC trades and sells different items


#define NPC_STATUS_IDLE 0
#define NPC_STATUS_WALKING 1
#define NPC_STATUS_PLAYER_NEARBY 2
#define NPC_STATUS_UNDER_THREAT 3 // <- TODO

#define NPC_TALK_DISTANCE 60

#define NPC_L_LEG_MI 0 // Righ leg index.
#define NPC_R_LEG_MI 1 // Left leg index.

#define NPC_RND_DEST_RADIUS 100.0 // Random destination radius.

struct npc_travel_t {
    Vector3 start;
    Vector3 dest;
    float  travelled;
    int    dest_reached;
};

struct npc_t {
    Model    model;
    Vector3  position;
    Vector3  rotation;
    Vector3  prev_position;
    int      player_nearby;
    int      active; // Is the npc currently in the world?    
    float    dist2player;

    // Used for rotating.
    Quaternion Q_now;
    Quaternion Q_target;
    Quaternion Q_prev;
    float angle_change;
   
    struct npc_travel_t travel;

    int      status;

    Matrix   leg_matrix[2];
};

struct state_t;


void setup_npc(struct state_t* gst, struct npc_t* npc);
void delete_npc(struct npc_t* npc);

void update_npc(struct state_t* gst, struct npc_t* npc);
void render_npc(struct state_t* gst, struct npc_t* npc);


#endif
