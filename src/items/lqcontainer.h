#ifndef LIQUID_CONTAINER_H
#define LIQUID_CONTAINER_H


// Content type
#define LQCONTENT_EMPTY -1
#define LQCONTENT_ENERGY_LIQUID 0
#define LQCONTENT_HEALTH_LIQUID 1


struct lqcontainer_t {

    float level;
    float capacity;

    int content_type;
};




#endif
