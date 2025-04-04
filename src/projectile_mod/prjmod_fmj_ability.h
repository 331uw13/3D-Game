#ifndef PRJMOD_FMJ_ABILITY_H
#define PRJMOD_FMJ_ABILITY_H

#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;
struct enemy_t;




int prjmod_fmj__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, struct enemy_t* ent);
int prjmod_fmj__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal);
int prjmod_fmj__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);
int prjmod_fmj__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);



#endif
