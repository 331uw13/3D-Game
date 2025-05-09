#include <stdio.h>

#include "inventory.h"
#include "state/state.h"


void inventory_init(struct inventory_t* inv) {
    inv->open = 0;

}



// 3D - Inventory.
void inventory_render(struct state_t* gst, struct inventory_t* inv) {
    Model* box = &gst->inventory_box_model;

    float px = gst->player.position.x;
    float py = gst->player.position.y;
    float pz = gst->player.position.z;


    Matrix icam_matrix = MatrixInvert(GetCameraMatrix(gst->player.cam));

    float box_scale = 0.3;

    struct light_t inv_light = (struct light_t) {
        .type = LIGHT_POINT,
        .enabled = 1,
        .color = (Color){ 240, 130, 30, 255 },
        .strength = 1.0,
        .radius = 2.0,
        .index = INVENTORY_LIGHT_ID,
        .position = gst->player.cam.position
    };

    set_light(gst, &inv_light, LIGHTS_UBO);

    float xf = 0.0;
    float yf = 0.0;

    float inv_depth = 4.0;
    float box_size = 2.0 * box_scale;

    // Figure out frustrum depth at inventory's depth.
    
    float aspect_ratio = (gst->screen_size.x / gst->cfg.res_div) / (gst->screen_size.y / gst->cfg.res_div);
    float fov_y = gst->player.cam.fovy * (M_PI/180.0);

    float fov_x = 2.0 * atan(tan(fov_y / 2.0) * aspect_ratio);
    float frustrum_width = (2.0 * inv_depth * tan(fov_x / 2.0)) / 2.0 - box_size;




    for(int y = 0;  y < INV_NUM_ROWS; y++) {
        for(int x = 0; x < INV_NUM_COLUMNS; x++) {

            Matrix offset    = MatrixTranslate(xf-frustrum_width, yf-1.0, -inv_depth);
            Matrix rotation  = MatrixRotateY(gst->time + xf*0.1);
            Matrix scale     = MatrixScale(box_scale, box_scale, box_scale);


            Matrix current = MatrixMultiply(rotation, MatrixMultiply(scale, offset));
            current = MatrixMultiply(current, icam_matrix);

            DrawMesh(
                box->meshes[0],
                box->materials[0],
                current);

            xf += box_size;
        }
        yf += box_size;
        xf = 0.0;
    }

}



