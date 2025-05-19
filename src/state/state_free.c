#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "state_free.h"

#include <raylib.h>
#include <rlgl.h>


static void state_delete_shaders(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_SHADERS)) { return; }
    for(int i = 0; i < MAX_SHADERS; i++) {
        UnloadShader(gst->shaders[i]);
    }

    printf("\033[35m -> Deleted Shaders.\033[0m\n");
}

static void state_delete_psystems(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_PSYSTEMS)) { return; }
    for(int i = 0; i < MAX_PSYSTEMS; i++) {
        delete_psystem(&gst->psystems[i]);
    }
   
    printf("\033[35m -> Deleted Particle systems.\033[0m\n");
}


static void state_delete_textures(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_TEXTURES)) { return; }
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->textures[i]);
    }

    UnloadTexture(gst->ssao_noise_tex);
    UnloadTexture(gst->colorpick_tex); 
    UnloadImage(gst->colorpick_img);
    printf("\033[35m -> Deleted Textures.\033[0m\n");
}

static void state_delete_sounds(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_SOUNDS)) { return; }
    if(gst->has_audio) {
        for(int i = 0; i < MAX_SOUNDS; i++) {
            UnloadSound(gst->sounds[i]);
        }    
    }
    if(IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    
    printf("\033[35m -> Deleted Sounds.\033[0m\n");
}

static void state_delete_enemy_models(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_ENEMY_MODELS)) { return; }
    for(int i = 0; i < MAX_ENEMY_MODELS; i++) {
        if(IsModelValid(gst->enemy_models[i])) {
            UnloadModel(gst->enemy_models[i]);
        }
    }

    printf("\033[35m -> Deleted Enemy models.\033[0m\n");
}

static void state_delete_render_targets(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_RENDERTARGETS)) { return; }
    UnloadRenderTexture(gst->env_render_target);
    UnloadRenderTexture(gst->env_render_downsample);
    UnloadRenderTexture(gst->inv_render_target);
    UnloadRenderTexture(gst->bloomtresh_target);
    UnloadRenderTexture(gst->ssao_target);
    UnloadRenderTexture(gst->ssao_final);

    for(int i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++) {
        UnloadRenderTexture(gst->bloom_downsamples[i]);
    }


    printf("\033[35m -> Deleted Render targets.\033[0m\n");
}

static void delete_gbuffer(struct gbuffer_t* gbuf) {
    rlUnloadFramebuffer(gbuf->framebuffer);
    rlUnloadTexture(gbuf->position_tex);
    rlUnloadTexture(gbuf->normal_tex);
    rlUnloadTexture(gbuf->difspec_tex);
    rlUnloadTexture(gbuf->depth_tex); 
    rlUnloadTexture(gbuf->depthbuffer);
}

static void state_delete_gbuffers(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_GBUFFERS)) { return; }
    delete_gbuffer(&gst->gbuffer);
    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        delete_gbuffer(&gst->shadow_gbuffers[i]);
    }

    printf("\033[35m -> Deleted Geometry buffers.\033[0m\n");
}

static void state_delete_ubos(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_UBOS)) { return; }
    for(int i = 0; i < MAX_UBOS; i++) {
        glDeleteBuffers(1, &gst->ubo[i]);
    }

    printf("\033[35m -> Deleted Uniform buffers.\033[0m\n");
}

static void state_delete_item_models(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_ITEM_MODELS)) { return; }
    for(int i = 0; i < MAX_ITEM_TYPES; i++) {
        UnloadModel(gst->item_models[i]);
    }

    printf("\033[35m -> Deleted Item Models.\033[0m\n");
}

static void state_delete_weapon_models(struct state_t* gst) {
    if(!(gst->init_flags & INITFLG_WEAPON_MODELS)) { return; }

    for(int i = 0; i < MAX_WEAPON_MODELS; i++) {
        delete_weapon_model(gst, &gst->weapon_models[i]);
    }

    printf("\033[35m -> Deleted Weapon Models.\033[0m\n");
}


void state_free_everything(struct state_t* gst) {
    delete_terrain(&gst->terrain);
    state_delete_shaders(gst);
    state_delete_psystems(gst);
    state_delete_textures(gst);
    state_delete_sounds(gst);
    
    state_delete_enemy_models(gst);
    state_delete_item_models(gst);

    state_delete_render_targets(gst);
    state_delete_gbuffers(gst);
    state_delete_ubos(gst);

   
    delete_player(gst, &gst->player);
    delete_npc(gst, &gst->npc);
    
    UnloadModel(gst->inventory_box_model);
    UnloadModel(gst->inventory_box_selected_model);
}




