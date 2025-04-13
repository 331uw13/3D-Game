#ifndef GAME_FOG_H
#define GAME_FOG_H


struct state_t;

struct fog_t {
    float density;
    int mode;
    Color color_top;
    Color color_bottom;
};


void set_fog_settings(struct state_t* gst, struct fog_t* fog);
void fog_blend(
        struct state_t* gst,
        struct fog_t* fog_target,
        float  blend, // 0.0 - 1.0
        struct fog_t* fog_from,
        struct fog_t* fog_to
        );


#endif
