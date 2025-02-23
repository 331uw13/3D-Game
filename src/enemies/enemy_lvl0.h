#ifndef ENEMY_LVL0
#define ENEMY_LVL0

#include "../entity.h"


struct state_t;


// PROJECTILE PARTICLE UPDATE ---
void enemy_lvl0_weapon_psystem_projectile_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);

void enemy_lvl0_weapon_psystem_projectile_pinit(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
);

void enemy_lvl0_update  (struct state_t* gst, struct entity_t* ent, int render_setting);
void enemy_lvl0_hit     (struct state_t* gst, struct entity_t* ent);
void enemy_lvl0_death   (struct state_t* gst, struct entity_t* ent);
void enemy_lvl0_created (struct state_t* gst, struct entity_t* ent);




#endif
