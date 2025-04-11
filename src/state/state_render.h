#ifndef STATE_RENDER_H
#define STATE_RENDER_H


#define RENDERPASS_RESULT 0
#define RENDERPASS_GBUFFER 1
#define RENDERPASS_SHADOWS 2

struct state_t;


// Prepare all shaders for the current render pass.
void prepare_renderpass(struct state_t* gst, int renderpass);


void state_render(struct state_t* gst);




#endif
