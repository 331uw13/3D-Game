#include <stdio.h>

#include "bloom.h"
#include "state/state.h"


void setup_bloom_targets(struct state_t* gst) {

    int dres_x = gst->res_x * gst->cfg.res_div;
    int dres_y = gst->res_y * gst->cfg.res_div;

    // Setup bloom downsample targets.
    int down_res_x = dres_x;
    int down_res_y = dres_y;
    for(int i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++) {

        down_res_x /= 1.2825;
        down_res_y /= 1.2825;

        printf("%s (bloom downsample res %i): %ix%i\n",
                __func__, i, down_res_x, down_res_y);
        
        gst->bloom_downsamples[i] = LoadRenderTexture(down_res_x, down_res_y);
        SetTextureFilter(gst->bloom_downsamples[i].texture, TEXTURE_FILTER_BILINEAR);
    }
}

void render_bloom(struct state_t* gst) {

    // Get bloom treshold.

    resample_texture(gst,
            gst->bloomtresh_target, // destination.
            gst->env_render_target, // source.
            gst->env_render_target.texture.width,
            gst->env_render_target.texture.height,
            gst->bloomtresh_target.texture.width,
            gst->bloomtresh_target.texture.height,
            BLOOM_TRESHOLD_SHADER
            );
    
    // Downsample bloom treshold
    resample_texture(gst,
            gst->bloom_downsamples[0], // destination.
            gst->bloomtresh_target, // source.
            gst->bloomtresh_target.texture.width,
           -gst->bloomtresh_target.texture.height,
            gst->bloom_downsamples[0].texture.width,
            gst->bloom_downsamples[0].texture.height,
            -1
            );
    
    const int last = NUM_BLOOM_DOWNSAMPLES-1;

    for(int i = 1; i < last; i++) {
        int p = i - 1; // Previous.

        resample_texture(gst,
                gst->bloom_downsamples[i],
                gst->bloom_downsamples[p],
                gst->bloom_downsamples[p].texture.width,
               -gst->bloom_downsamples[p].texture.height,
                gst->bloom_downsamples[i].texture.width,
                gst->bloom_downsamples[i].texture.height,
                -1
                );
    }


    // Apply blur to very low resolution texture.

    Vector2 size = (Vector2){ 
        gst->bloom_downsamples[1].texture.width,
        gst->bloom_downsamples[1].texture.height
    };
    shader_setu_vec2(gst, BLOOM_BLUR_SHADER, U_SCREEN_SIZE, &size);
 
    resample_texture(gst,
            gst->bloom_downsamples[last],
            gst->bloom_downsamples[last-1],
            gst->bloom_downsamples[last-1].texture.width,
           -gst->bloom_downsamples[last-1].texture.height,
            gst->bloom_downsamples[last].texture.width,
            gst->bloom_downsamples[last].texture.height,
            BLOOM_BLUR_SHADER
            );


    // "Upscale" blurred very low resolution texture.

    resample_texture(gst,
            gst->bloomtresh_target,
            gst->bloom_downsamples[last],
            gst->bloom_downsamples[last].texture.width,
            gst->bloom_downsamples[last].texture.height,
            gst->bloomtresh_target.texture.width,
            gst->bloomtresh_target.texture.height,
            -1
            );
    
    // Downsample more and apply blur.

    /*
    Vector2 size = (Vector2){ 
        gst->bloom_downsamples[1].texture.width,
        gst->bloom_downsamples[1].texture.height
    };
    shader_setu_vec2(gst, BLOOM_BLUR_SHADER, U_SCREEN_SIZE, &size);
    
    resample_texture(
            gst,
            gst->bloom_downsamples[1],
            gst->bloom_downsamples[0],
            gst->bloom_downsamples[0].texture.width,
           -gst->bloom_downsamples[0].texture.height,
            gst->bloom_downsamples[1].texture.width,
            gst->bloom_downsamples[1].texture.height,
            BLOOM_BLUR_SHADER
            );


    // Upsample blurred bloom treshold.
    resample_texture(gst,
            gst->bloomtresh_target,
            gst->bloom_downsamples[1],
            gst->bloom_downsamples[1].texture.width,
            gst->bloom_downsamples[1].texture.height,
            gst->bloomtresh_target.texture.width,
            gst->bloomtresh_target.texture.height,
            -1 // No special shader needed.
            );
    */
    

}




