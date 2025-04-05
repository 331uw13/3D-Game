#ifndef PRJMOD_PRJMOD_CLOUDBURST_H
#define PRJMOD_PRJMOD_CLOUDBURST_H

#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;
struct enemy_t;
struct hitbox_t;



void prjmod_cloudburst__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);
void prjmod_cloudburst__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);

void prjmod_cloudburst__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal);
int prjmod_cloudburst__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part,
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage);



#endif
