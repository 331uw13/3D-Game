#ifndef PLAYER_H
#define PLAYER_H


struct state_t;


struct player_t {

    Vector3  position;
    Vector3  size;
    BoundingBox boundingbox;
    Vector3  velocity;
    float    walkspeed;
    float    run_mult;
    float    friction;
    float    jump_force;
    float    gravity;
    int      onground;
};


void init_player_struct(struct player_t* p);

// updates player's variables. not movement
// for movement see (input.c)
void update_player(struct state_t* gst, struct player_t* p);



#endif
