#include <raylib.h>

#include "state.h"
#include "player.h"


void init_player_struct(struct player_t* p) {

    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    p->size = (Vector3){ 1.0, 2.0, 1.0 };
    p->velocity = (Vector3){ 0.0, 0.0, 0.0 };
    p->walkspeed = 0.8;
    p->jump_force = 0.128;
    p->gravity = 0.6;
    p->run_mult = 2.3;
    p->onground = 1;
    p->friction = 0.075;


}



void update_player(struct state_t* gst, struct player_t* p) {
    
    // update the bounding box.
    p->boundingbox = (BoundingBox) {
        (Vector3) {            
            p->position.x - p->size.x/2,
            p->position.y - p->size.y/2,
            p->position.z - p->size.z/2,
        },
        (Vector3) {
            p->position.x + p->size.x/2,
            p->position.y + p->size.y/2,
            p->position.z + p->size.z/2,
        }
    };



}



