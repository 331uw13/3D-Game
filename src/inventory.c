#include <stdio.h>

#include "inventory.h"
#include "state/state.h"

#include <rlgl.h>


void inventory_init(struct inventory_t* inv) {
    inv->open = 0;

    for(int i = 0; i < INV_SIZE; i++) {
        inv->items[i].empty = 1;
        inv->items[i].inv_index = -1;
    }

}



// 3D - Inventory.
void inventory_render(struct state_t* gst, struct inventory_t* inv) {
    Model* box = &gst->inventory_box_model;

    
    Matrix icam_matrix = MatrixInvert(GetCameraMatrix(gst->player.cam));

    float box_scale = 0.4;

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

    float x_zeroto_pi = 0.0;
    float x_zeroto_pi_inc = M_PI / (float)INV_NUM_COLUMNS + 0.3;

    Ray mouse_ray = GetScreenToWorldRay(GetMousePosition(), gst->player.cam);
   

    const Color light_color = (Color) { 198, 220, 230, 255 };
    const Color light_color_empty = (Color){ light_color.r/3, light_color.g/3, light_color.b/3, 255 };

    for(int y = 0;  y < INV_NUM_ROWS; y++) {
        for(int x = 0; x < INV_NUM_COLUMNS; x++) {

            float depth_offset = sin(x_zeroto_pi*0.6);

            x_zeroto_pi += x_zeroto_pi_inc;

            Matrix offset = MatrixTranslate(
                    xf-(frustrum_width/2)-box_size/2,
                    yf-1.0,
                    -(inv_depth + depth_offset));

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

            Vector3 box_center = (Vector3){ current.m12, current.m13, current.m14 };
            RayCollision rayhit = GetRayCollisionSphere(mouse_ray, box_center, 0.25);

            float item_scale = 0.3;
            Matrix item_matrix = MatrixMultiply(MatrixScale(item_scale, item_scale, item_scale), current);
            item_matrix = MatrixMultiply(MatrixRotateXYZ(
                        (Vector3){
                            0.57,
                            gst->time + (x+y),
                            0.32
                        }), item_matrix);

            struct item_t* item = &inv->items[x + y * INV_NUM_COLUMNS];


            { // TODO: This dont need to be updated every frame.
              //         ^ Only position.

                Color lcolor = light_color;

                
                if(inv->selected_item) {
                    if(inv->selected_item->inv_index == item->inv_index) {
                        lcolor = (Color){ 60, 250, 255, 255 };
                    }
                }

                Matrix light_matrix = MatrixMultiply(
                        MatrixTranslate(0.0, 0.9, 0.0), // Light position offset
                        current
                        );

                struct light_t inv_light = (struct light_t) {
                    .type = LIGHT_POINT,
                    .enabled = 1,
                    .color = (item->inv_index < 0) ? light_color_empty : lcolor,
                    .strength = 0.42,
                    .radius = 1.2,
                    .index = INVENTORY_LIGHT_ID,
                    .position = (Vector3){ light_matrix.m12, light_matrix.m13, light_matrix.m14 }
                };
                set_light(gst, &inv_light, LIGHTS_UBO);
            }

            if(!item->empty && item->inv_index >= 0) {
                if(item->is_weapon_item) {
                    render_weapon_model(gst, &item->weapon_model, item_matrix);
                }
                else {
                    DrawMesh(item->modelptr->meshes[0],
                            item->modelptr->materials[0],
                            item_matrix);
                }
            }

            DrawMesh(
                box->meshes[0],
                box->materials[0],
                current);

            if(rayhit.hit) {
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    inv->selected_item = item;
                }
                DrawMesh(
                        gst->inventory_box_selected_model.meshes[0],
                        gst->inventory_box_selected_model.materials[0],
                        current
                        );
            }
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
        if(inv->items[i].inv_index >= 0) {
            continue;
        }
        index = i;
        break;
    }

    return index;
}


void inventory_move_item(struct inventory_t* inv, struct item_t* item, int index) {
   
    if(index <= INV_INDEX_NEXT_FREE) {
        index = find_free_space(inv);
        if(index < 0) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Inventory is full.\033[0m\n",
                    __func__);
            return;
        }
    }

    item->lifetime = 0.0;
    item->inv_index = index;
    inv->items[index] = *item;
    

}



