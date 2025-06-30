#ifndef LIQUID_CONTAINER_H
#define LIQUID_CONTAINER_H

#include <raylib.h>

// Content type
// Fractal trees use these too.
#define LQCONTENT_EMPTY -1
#define LQCONTENT_ENERGY 0
#define LQCONTENT_HEALTH 1
#define MAX_LQCONTENT_TYPES 2

struct state_t;

struct lqcontainer_t {

    float level;
    float capacity;

    int content_type;
};


void render_lqcontainer(struct state_t* gst, struct lqcontainer_t* lqcontainer, Matrix transform);

#endif
