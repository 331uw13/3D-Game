#ifndef PRJMOD_TEST_H
#define PRJMOD_TEST_H

#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;
struct enemy_t;




int prjmod_test__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, struct enemy_t* ent);
int prjmod_test__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal);
int prjmod_test__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);
int prjmod_test__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);



#endif
