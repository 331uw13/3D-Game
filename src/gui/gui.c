#include "gui.h"



int mouse_on_rect(Vector2 rect_pos, Vector2 rect_size) {
    Vector2 mouse = GetMousePosition();

    return (
            (mouse.x > rect_pos.x) && (mouse.x < (rect_pos.x+rect_size.x)) &&
            (mouse.y > rect_pos.y) && (mouse.y < (rect_pos.y+rect_size.y))
            );

}


