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

    inv->selected_item = NULL;
    inv->hovered_item = NULL;
}

void inventory_open_event(struct state_t* gst, struct inventory_t* inv) {
    inv->open = 1;

    inv->light = (struct light_t) {
        .type = LIGHT_POINT,
        .enabled = 1,
        .color = (Color){ 255, 255, 255, 255 },
        .strength = 0.4,
        .radius = 0.7,
        .index = INVENTORY_LIGHT_ID,
        .position = (Vector3){ 0 }
    };
    set_light(gst, &inv->light, LIGHTS_UBO);
    schedule_new_render_dist(gst, MIN_RENDERDIST);
}

void inventory_close_event(struct state_t* gst, struct inventory_t* inv) {
    inv->open = 0;

    inv->light.enabled = 0;
    set_light(gst, &inv->light, LIGHTS_UBO);
    schedule_new_render_dist(gst, gst->old_render_dist);
}

// 3D - Inventory.
void inventory_render(struct state_t* gst, struct inventory_t* inv) {
    Model* box = &gst->inventory_box_model;

    
    Matrix icam_matrix = MatrixInvert(GetCameraMatrix(gst->player.cam));



    float xf = 0.0;  // Used for 'current box X'
    float yf = 0.0;  // Used for 'current box Y'

    const float box_scale = 0.4;
    const float inv_depth = 4.0;
    const float box_size = 2.0 * box_scale;
    const float box_padding = 0.165;

    // Figure out frustrum depth at inventory's box depth.
    
    float aspect_ratio = (gst->screen_size.x / gst->cfg.res_div) / (gst->screen_size.y / gst->cfg.res_div);
    float fov_y = gst->player.cam.fovy * (M_PI/180.0);

    float fov_x = 2.0 * atan(tan(fov_y / 2.0) * aspect_ratio);
    float frustrum_width = (2.0 * inv_depth * tan(fov_x / 2.0)) / 2.0 - box_size;


    float x_zeroto_pi = 0.0;
    float x_zeroto_pi_inc = M_PI / (float)INV_NUM_COLUMNS + 0.3;

    Ray mouse_ray = GetScreenToWorldRay(GetMousePosition(), gst->player.cam);
   

    const Color light_color = (Color) { 198, 220, 230, 255 };
    const Color light_color_empty = (Color){ light_color.r/3, light_color.g/3, light_color.b/3, 255 };

    inv->hovered_item = NULL;



    Vector3 light_pos = 
        Vector3Add(gst->player.cam.position,
                Vector3Multiply(mouse_ray.direction, (Vector3){ 4, 4, 4 })); 
    set_light_pos(gst, &inv->light, light_pos);



    for(int y = 0;  y < INV_NUM_ROWS; y++) {
        for(int x = 0; x < INV_NUM_COLUMNS; x++) {

            float depth_offset = sin(x_zeroto_pi*0.6);

            x_zeroto_pi += x_zeroto_pi_inc;

            Matrix offset = MatrixTranslate(
                    xf-(frustrum_width/2)-box_size,
                    yf-1.5,
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
            
            // Item rotation.
            item_matrix = MatrixMultiply(MatrixRotateXYZ(
                        (Vector3){
                            0.57,
                            gst->time + (x+y),
                            0.32
                        }), item_matrix);

            struct item_t* item = &inv->items[x + y * INV_NUM_COLUMNS];

            // Render the item in current box.
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

            // Box.
            DrawMesh(
                box->meshes[0],
                box->materials[0],
                current);

            // Mouse is on box, handle input.
            if(rayhit.hit) { 
                inv->hovered_item = item;

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    inv->selected_item = item;
                }

                if(gst->mouse_double_click) {
                    player_change_holding_item(gst, &gst->player, item);
                }


                // Render selected box indicator.
                Matrix selected_current = MatrixMultiply(
                        MatrixRotateXYZ((Vector3){ gst->time, 0, gst->time*3.0 }), current);
                DrawMesh(
                        gst->inventory_box_selected_model.meshes[0],
                        gst->inventory_box_selected_model.materials[0],
                        selected_current
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



