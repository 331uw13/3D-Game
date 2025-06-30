#include <stdio.h>

#include "item_combine.h"
#include "item.h"
#include "../state/state.h"




void combine__weapon_model__lqcontainer(struct state_t* gst, struct item_t* item_A, struct item_t* item_B) {

    struct weapon_model_t* weapon_model = NULL;
    struct lqcontainer_t* lqcontainer = NULL;

    if((item_A->type == ITEM_WEAPON_MODEL)
    && (item_B->type == ITEM_LQCONTAINER)) {
        weapon_model = &item_A->weapon_model;
        lqcontainer = &item_B->lqcontainer;
    }
    else
    if((item_A->type == ITEM_LQCONTAINER)
    && (item_B->type == ITEM_WEAPON_MODEL)) {
        lqcontainer = &item_A->lqcontainer;
        weapon_model = &item_B->weapon_model;
    }


    if(lqcontainer->level <= 0.0) {
        printf("Liquid Container %p is empty\n", lqcontainer);
        return;
    }


    if(lqcontainer->content_type != LQCONTENT_ENERGY) {
        printf("Liquid Container %p has wrong content\n", lqcontainer);
        return;
    }


    weapon_model->stats.lqmag.ammo_level += lqcontainer->level;
    lqcontainer->level = 0;

    weapon_model->stats.lqmag.ammo_level
        = CLAMP(weapon_model->stats.lqmag.ammo_level, 0, weapon_model->stats.lqmag.capacity);

    // If the liquid manazine ammo level will "overflow"
    // add remaining back to the container.
    if(weapon_model->stats.lqmag.capacity
    < weapon_model->stats.lqmag.ammo_level) {
        float difference = weapon_model->stats.lqmag.ammo_level - weapon_model->stats.lqmag.capacity;
        lqcontainer->level += difference;
    }
}



