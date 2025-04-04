#include <stdio.h>

#include "prjmod_test.h"
#include "../state.h"


int prjmod_test__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, struct enemy_t* ent) {
    printf("'%s'\n", __func__);
    
    return 0;
}


int prjmod_test__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {
    printf("'%s'\n", __func__);


    return 0;
}


int prjmod_test__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
    printf("'%s'\n", __func__);


    return 0;
}



int prjmod_test__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {

    //printf("'%s' %f\n", __func__, gst->time);

    return 0;
}



