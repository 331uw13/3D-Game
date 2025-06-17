
#include "lqcontainer.h"
#include "../state/state.h"



void render_lqcontainer(struct state_t* gst, struct lqcontainer_t* lqcontainer, Matrix transform) {

    DrawMesh(
            gst->item_models[ITEM_LQCONTAINER].meshes[0],
            gst->item_models[ITEM_LQCONTAINER].materials[0],
            transform
            );

    DrawMesh(
            gst->item_models[ITEM_LQCONTAINER].meshes[1],
            gst->item_models[ITEM_LQCONTAINER].materials[0],
            transform
            );

    DrawMesh(
            gst->item_models[ITEM_LQCONTAINER].meshes[2],
            gst->item_models[ITEM_LQCONTAINER].materials[0],
            transform
            );


    shader_setu_float(gst, ENERGY_LIQUID_SHADER, U_ENERGY_CONTAINER_LEVEL,
            &lqcontainer->level);
    
    shader_setu_float(gst, ENERGY_LIQUID_SHADER, U_ENERGY_CONTAINER_CAPACITY,
            &lqcontainer->capacity);

    Color liquid_color = (Color) {
        20, 255, 255, 255
    };

    shader_setu_color(gst, ENERGY_LIQUID_SHADER, U_ENERGY_COLOR,
            &liquid_color);

    DrawMesh(
            gst->item_models[ITEM_LQCONTAINER].meshes[3],
            gst->item_models[ITEM_LQCONTAINER].materials[1],
            transform
            );
}

