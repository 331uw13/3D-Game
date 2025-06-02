#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "platform/platform.h"

#define CFG_SSAO_QLOW 0
#define CFG_SSAO_QMED 1
#define CFG_SSAO_QHIGH 2


// Game state config.
struct config_t {
    int fullscreen;
    int resolution_x;
    int resolution_y;
    int ssao_quality;
    int ssao_kernel_samples;
    float res_div; // Resolution divisor (see game.cfg for detailed info)
    int render_dist;
};


// TODO: Instead of returning a buffer for every variable type.
// And other types exists too.
// function like 'read_cfgvar(cfgfile, "name", &ptr, VTYPE_FLOAT)'

int read_cfgvar(platform_file_t* cfgfile, const char* name, char* outbuf, size_t outbuf_size);




#endif
