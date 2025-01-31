#ifndef PROJECTILE_LIGHTS_H
#define PROJECTILE_LIGHTS_H

#include <raylib.h>

struct state_t;

void enable_projectile_light(struct state_t* gst, unsigned int index, Vector3 initpos);
void disable_projectile_light(struct state_t* gst, unsigned int index);



#endif
