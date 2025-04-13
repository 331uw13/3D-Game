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


#endif
