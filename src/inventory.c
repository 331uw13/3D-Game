#include <stdio.h>

#include "inventory.h"
#include "state/state.h"


void inventory_init(struct inventory_t* inv) {
    inv->open = 0;

    for(int i = 0; i < INV_SIZE; i++) {
        inv->items[i].empty = 1;
    }

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
        .color = (Color){ 240, 150, 30, 255 },
        .strength = 0.86,
        .radius = 3.0,
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

    float box_padding = 0.1;
    float total_boxes_width = (INV_NUM_COLUMNS * (box_size + box_padding));

    float x_zeroto_pi = 0.0;
    float x_zeroto_pi_inc = M_PI / (float)INV_NUM_COLUMNS + 0.3;

    for(int y = 0;  y < INV_NUM_ROWS; y++) {
        for(int x = 0; x < INV_NUM_COLUMNS; x++) {
            
            float depth_offset = 1.0 - (0.5+0.5*cos(x_zeroto_pi*1.3));
            depth_offset = pow(depth_offset, 0.5);
            x_zeroto_pi += x_zeroto_pi_inc;

            Matrix offset = MatrixTranslate(
                    xf-(frustrum_width/2)-box_size/2,
                    yf-1.0,
                    -(inv_depth + depth_offset*0.35));

            float roty = 0.0;
            float rotx = 0.0;

            // Calculate rotation angle for box.
            // inverted camera matrix has to be multiplied with offset first
            // to get box position.
            {
                Matrix tmp = MatrixMultiply(icam_matrix, offset);
                Vector3 angles = get_rotation_yz(
                        gst->player.cam.position, (Vector3){ tmp.m12, tmp.m13, tmp.m14 });
                roty = angles.y + M_PI/2;
                rotx = -angles.z;
            }

            Matrix rotation  = MatrixIdentity();
            rotation = MatrixMultiply(rotation, MatrixRotateX(rotx));
            rotation = MatrixMultiply(rotation, MatrixRotateY(roty));

            Matrix scale = MatrixScale(box_scale, box_scale, box_scale);
            Matrix current = MatrixMultiply(rotation, MatrixMultiply(scale, offset));
            current = MatrixMultiply(current, icam_matrix);

            DrawMesh(
                box->meshes[0],
                box->materials[0],
                current);

            xf += box_size + box_padding;
        }
        x_zeroto_pi = 0.0;
        yf += box_size + box_padding;
        xf = 0.0;
    }
}

static int find_free_space(struct inventory_t* inv) {
    int index = -1;

    for(int i = 0; i < INV_SIZE; i++) {
        if(!inv->items[i].empty) {
            continue;
        }
        index = i;
        break;
    }

    return index;
}


void inventory_add_item(struct inventory_t* inv, struct item_t* item, int setting) {
   
    int index = find_free_space(inv);
    if(index < 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Inventory is full.\033[0m\n",
                __func__);
        return;
    }

    if(setting == INV_MOVE_SOURCE) {
         
    }
    else
    if(setting == INV_COPY_SOURCE) {
        
    }

}



