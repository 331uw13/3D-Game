#include <stdio.h>

#include "inventory.h"
#include "state/state.h"
#include "items/item.h"

#include <rlgl.h>



void inventory_init(struct inventory_t* inv) {
    inv->open = 0;

    for(int i = 0; i < INV_SIZE; i++) {
        inv->items[i] = get_empty_item();
        inv->items[i].empty = 1;
        //inv->items[i].inv_index = -1;
    }

    inv->selected_item = NULL;
    inv->hovered_item = NULL;
    inv->mouse_down_timer = 0;
    inv->item_drag = 0;

    printf("%s\n", __func__);
}

void inventory_open(struct state_t* gst, struct inventory_t* inv) {
    if(inv->open) {
        return;
    }

    inv->light = add_light(gst, (struct light_t) {
        .color = (Color){ 230, 220, 210, 255 },
        .radius = 1.0,
        .position = (Vector3){ 0 },
        .strength = 1.0,
        .preserve = 0
    },
    NEVER_OVERWRITE);

    EnableCursor();
    inv->open = 1;
    //schedule_new_render_dist(gst, MIN_RENDERDIST);
}

void inventory_close(struct state_t* gst, struct inventory_t* inv) {
    if(!inv->open) {
        return;
    }

    remove_light(gst, inv->light);
    
    DisableCursor();
    inv->open = 0;
    //schedule_new_render_dist(gst, gst->old_render_dist);
}

// 3D - Inventory.
void inventory_render(struct state_t* gst, struct inventory_t* inv) {
    Model* box = &gst->inventory_box_model;

    if(!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        
        if(inv->item_drag && inv->selected_item) {

            if(inv->hovered_item
            && !inv->hovered_item->empty) { /* Try to combine items */
                printf("%s -> %s\n", inv->selected_item->info->name, inv->hovered_item->info->name);
                
                combine_items(gst, inv->selected_item, inv->hovered_item);

            }
            else
            if(inv->hovered_item) { /* Move item */
                printf("<empty>\n");

                *inv->hovered_item = *inv->selected_item;
                *inv->selected_item = get_empty_item();
            }



        }
   
        inv->item_drag = 0;
        inv->mouse_down_timer = 0;
    }
    
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
   

    inv->hovered_item = NULL;
    inv->light->position = Vector3Add(mouse_ray.position, Vector3Scale(mouse_ray.direction, 2.65));


    shader_setu_float(gst, INVBOX_BACKGROUND_SHADER, U_TIME, &gst->time);

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

            // This is so the box always faces the camera correctly.
            Matrix rotation  = MatrixIdentity();
            rotation = MatrixMultiply(rotation, MatrixRotateX(rotx));
            rotation = MatrixMultiply(rotation, MatrixRotateY(roty));

            // Box scale.
            Matrix scale = MatrixScale(box_scale, box_scale, box_scale);
            Matrix current = MatrixMultiply(rotation, MatrixMultiply(scale, offset));
            current = MatrixMultiply(current, icam_matrix);

            // Raycast sphere at the same place as box to see if mouse is on it.
            Vector3 box_center = (Vector3){ current.m12, current.m13, current.m14 };
            RayCollision rayhit = GetRayCollisionSphere(mouse_ray, box_center, 0.42);

            float item_scale = 0.3;
            Matrix item_matrix = MatrixMultiply(MatrixScale(item_scale, item_scale, item_scale), current);
            
            struct item_t* item = &inv->items[x + y * INV_NUM_COLUMNS];
 
            // Set dragged item to mouse.
            if(inv->item_drag
            && (inv->selected_item == item)) {
                Vector3 item_mouse_pos 
                    = Vector3Add(mouse_ray.position, Vector3Scale(mouse_ray.direction, 3.8));

                item_mouse_pos = Vector3Subtract(item_mouse_pos, (Vector3){
                    item_matrix.m12, item_matrix.m13, item_matrix.m14
                });

                Matrix mouse_offset = MatrixTranslate(item_mouse_pos.x, item_mouse_pos.y, item_mouse_pos.z);
                item_matrix = MatrixMultiply(item_matrix, mouse_offset);
            }   

            // Item rotation.
            item_matrix = MatrixMultiply(MatrixRotateXYZ(
                        (Vector3){
                            0.57,
                            gst->time + (x+y),
                            0.32
                        }), item_matrix);



            // Render the item in current box.
            if(!item->empty && item->inv_index >= 0) {
                switch(item->type) {
                    case ITEM_WEAPON_MODEL:
                        render_weapon_model(gst, &item->weapon_model, item_matrix);
                        break;

                    case ITEM_LQCONTAINER:
                        render_lqcontainer(gst, &item->lqcontainer, item_matrix);
                        break;

                    default:
                        render_item(gst, item, item_matrix);
                        break;
                }
            }


            Matrix bg_matrix = MatrixMultiply(MatrixRotateXYZ((Vector3){ M_PI/2.0, 0, 0 }), current);
            bg_matrix = MatrixMultiply(MatrixTranslate(0,-1.5,0), bg_matrix);

            // Box background. Indicates the item rarity.
            
            Color rarity_color = (Color){ 0, 0, 0, 0 };
            if(!item->empty) {
                rarity_color = get_item_rarity_color(item);
            }
            shader_setu_color(gst, INVBOX_BACKGROUND_SHADER, U_ITEM_RARITY_COLOR, &rarity_color);

            DrawMesh(
                    gst->inventory_box_background.meshes[0],
                    gst->inventory_box_background.materials[0],
                    bg_matrix
                    );

            DrawMesh(
                box->meshes[0],
                box->materials[0],
                current);

            // Mouse is on box, handle input.
            if(rayhit.hit) { 
                inv->hovered_item = item;

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if(IsKeyDown(KEY_LEFT_ALT) && !item->empty) {
                        drop_item(gst, FIND_ITEM_CHUNK, gst->player.position, item);
                        *item = get_empty_item();
                    }
                    else {
                        inv->selected_item = item;
                    }
                }

                if(gst->mouse_double_click) {
                    player_change_holding_item(gst, &gst->player, item);
                }

                if(!inv->item_drag 
                && inv->selected_item
                && !inv->selected_item->empty
                && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    inv->mouse_down_timer += gst->dt;
                    if(inv->mouse_down_timer > 0.1) {
                        inv->item_drag = 1;
                    }
                }

                // Selected box indicator.
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


void inventory_move_item(struct state_t* gst, struct inventory_t* inv, struct item_t* item, int index) {
   
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



