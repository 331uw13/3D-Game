
#include <math.h>

#include "perlin_noise.h"
#include "util.h"


static int perm_table[512] = { 0 };



static float grad3d(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : ((h == 12) || (h == 14)) ? x : z;
    return (((h & 1) == 0) ? u : -u) + ((h & 2) == 0) ? v : -v;
}

static float grad2d(int hash, float x, float y) {
    return ((hash & 1) == 0 ? x : -x) + ((hash & 2) == 0 ? y : -y);
}

static float fade(float t) {
    return (t * t * t * (t * (t * 6.0 - 15.0) + 10.0));
}


float perlin_noise_3D(float x, float y, float z) {

    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);


    int A  = perm_table[X]+Y;
    int AA = perm_table[A]+Z;
    int AB = perm_table[A+1]+Z;
    int B  = perm_table[X+1]+Y;
    int BA = perm_table[B]+Z;
    int BB = perm_table[B+1]+Z;


    return lerp(w, lerp(v, lerp(u, grad3d(perm_table[AA  ], x  , y  , z   ),
                               grad3d(perm_table[BA  ], x-1, y  , z   )), 
                       lerp(u, grad3d(perm_table[AB  ], x  , y-1, z   ),  
                               grad3d(perm_table[BB  ], x-1, y-1, z   ))),
               lerp(v, lerp(u, grad3d(perm_table[AA+1], x  , y  , z-1 ),  
                               grad3d(perm_table[BA+1], x-1, y  , z-1 )), 
                       lerp(u, grad3d(perm_table[AB+1], x  , y-1, z-1 ),
                               grad3d(perm_table[BB+1], x-1, y-1, z-1 ))));

}

float perlin_noise_2D(float x, float y) {
    
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;

    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);
    int A = (perm_table[X  ] + Y) & 255;
    int B = (perm_table[X+1] + Y) & 255;

    return lerp(v, lerp(u, grad2d(perm_table[A],   x, y), 
                           grad2d(perm_table[B  ], x-1, y)),
                   lerp(u, grad2d(perm_table[A+1], x, y-1), 
                           grad2d(perm_table[B+1], x-1, y-1)));

}


float fbm_2D(float x, float y, int octave) {
    
    float f = 0.0;
    float w = 2.0;

    for(int i = 0; i < octave; i++) {
        f += w * perlin_noise_2D(x, y);
        x *= 2.0;
        y *= 2.0;

        w *= 0.5;
    }

    return f;
}



void init_perlin_noise() {
    for(int i = 0; i < 256; i++) {
        perm_table[i+256] = perm_table[i] = permutation[i];
    }
}

