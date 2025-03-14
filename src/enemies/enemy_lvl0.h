#ifndef ENEMY_TYPE_LVL0_H
#define ENEMY_TYPE_LVL0_H

#include "../enemy.h"


// Index for 'enemy->matrix'.  'MI' short for "Matrix Index".
#define ENEMY_LVL0_BODY_MI 0
#define ENEMY_LVL0_JOINT_MI 1
#define ENEMY_LVL0_LEG_MI 2


struct state_t;

void enemy_lvl0_hit(struct state_t* gst, struct enemy_t* ent, 
        Vector3 hit_position, Vector3 hit_direction);
void enemy_lvl0_update  (struct state_t* gst, struct enemy_t* ent);
void enemy_lvl0_render  (struct state_t* gst, struct enemy_t* ent);
void enemy_lvl0_death   (struct state_t* gst, struct enemy_t* ent);
void enemy_lvl0_created (struct state_t* gst, struct enemy_t* ent);




#endif
