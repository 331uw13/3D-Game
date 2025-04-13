#include <stdio.h>
#include <math.h>

#include "state/state.h"
#include "fog.h"


void set_fog_settings(struct state_t* gst, struct fog_t* fog) {
    glBindBuffer(GL_UNIFORM_BUFFER, gst->ubo[FOG_UBO]);

    float top_color4f[4] = {
        (float)fog->color_top.r / 255.0,
        (float)fog->color_top.g / 255.0,
        (float)fog->color_top.b / 255.0,
        1.0
    };

    float bottom_color4f[4] = {
        (float)fog->color_bottom.r / 255.0,
        (float)fog->color_bottom.g / 255.0,
        (float)fog->color_bottom.b / 255.0,
        1.0
    };

    float settings[4] = { 0 };


    if(fog->mode == FOG_MODE_RENDERDIST) {
        float test = 1.0 / (gst->render_dist-gst->render_dist/2.0);
        test = pow(test, exp(test));
        settings[0] = test;
    }




    //printf("'%s': New fog density = %f\n", __func__, settings[0]);

    size_t offset;
    size_t size = sizeof(float)*4;

    offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, settings);

    offset = 16;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, top_color4f);

    offset = 32;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, bottom_color4f);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    /*
    gst->render_bg_color = (Color) {
        gst->fog.color_far.r * 0.6,
        gst->fog.color_far.g * 0.6,
        gst->fog.color_far.b * 0.6,
        255
    };


    int max_far = 100;
    gst->fog.color_far.r = CLAMP(gst->fog.color_far.r, 0, max_far);
    gst->fog.color_far.g = CLAMP(gst->fog.color_far.g, 0, max_far);
    gst->fog.color_far.b = CLAMP(gst->fog.color_far.b, 0, max_far);

    float near_color[4] = {
        (float)gst->fog.color_near.r / 255.0,
        (float)gst->fog.color_near.g / 255.0,
        (float)gst->fog.color_near.b / 255.0,
        1.0
    };

    float far_color[4] = {
        (float)gst->fog.color_far.r / 255.0,
        (float)gst->fog.color_far.g / 255.0,
        (float)gst->fog.color_far.b / 255.0,
        1.0
    };

    size_t offset;
    size_t size;
    size = sizeof(float)*4;

    // DENSITY
   
    float density[4] = {
        // Convert fog density to more friendly scale.
        // Otherwise it is very exponental with very very samll numbers.
        0.0, //map(log(CLAMP(gst->fog_density, FOG_MIN, FOG_MAX)), FOG_MIN, FOG_MAX, 0.0015, 0.02),
        0.0,
        0.0,  // Maybe adding more settings later.
        0.0
    };


    offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, density);

    // NEAR COLOR
    offset = 16;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, near_color);

    // FAR COLOR
    offset = 16*2;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, far_color);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
     */
}

void fog_blend(struct state_t* gst, 
        struct fog_t* fog_target,
        float  blend,
        struct fog_t* fog_from,
        struct fog_t* fog_to
){

    fog_target->color_top = ColorLerp(fog_from->color_top, fog_to->color_top, blend);
    fog_target->color_bottom = ColorLerp(fog_from->color_bottom, fog_to->color_bottom, blend);
    set_fog_settings(gst, fog_target);


}


