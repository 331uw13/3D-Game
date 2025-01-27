#include <math.h>

#include "util.h"
#define _2PI 6.28318


void rainbow_palette(float t, unsigned char* red, unsigned char* grn, unsigned char* blu) {
    // for more information about this check out:
    // https://iquilezles.org/articles/palettes/

    float rf = (float)*red / 255.0;
    float gf = (float)*grn / 255.0;
    float bf = (float)*blu / 255.0;

    *red = (unsigned char)((0.5+0.5 * cos(_2PI * t)) * 255.0);
    *grn = (unsigned char)((0.5+0.5 * cos(_2PI * (t+0.33))) * 255.0);
    *blu = (unsigned char)((0.5+0.5 * cos(_2PI * (t+0.67))) * 255.0);



}


