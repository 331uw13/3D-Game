#ifndef ENEMY_TYPE_LVL1_H
#define ENEMY_TYPE_LVL1_H

#include "../enemy.h"


// Index for 'enemy->matrix'.  'MI' short for "Matrix Index".
#define ENEMY_LVL1_BODY_MI 0
#define ENEMY_LVL1_JOINT_MI 1
#define ENEMY_LVL1_LEG_MI 2


struct state_t;

void enemy_lvl1_hit(struct state_t* gst, struct enemy_t* ent, 
        Vector3 hit_position, Vector3 hit_direction);
void enemy_lvl1_update  (struct state_t* gst, struct enemy_t* ent);
void enemy_lvl1_render  (struct state_t* gst, struct enemy_t* ent);
void enemy_lvl1_death   (struct state_t* gst, struct enemy_t* ent);
void enemy_lvl1_created (struct state_t* gst, struct enemy_t* ent);




#endif
