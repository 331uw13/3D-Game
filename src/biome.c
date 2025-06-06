#include <stdio.h>
#include <stdlib.h>

#include "state/state.h"
#include "biome.h"




void setup_biomes(struct state_t* gst) {

    // COMFY BIOME
    struct biome_t* comfy_biome = &gst->terrain.biomedata[BIOMEID_COMFY];
    *comfy_biome = (struct biome_t) {
        .id = BIOMEID_COMFY,
        .fog = (struct fog_t) {
            .mode = FOG_MODE_RENDERDIST,
            .color_top    = (Color){ 15, 60, 60, 255 },
            .color_bottom = (Color){ 20, 80, 80, 255 },
            .density = 0 // Density is ignored when fog mode is 'FOG_MODE_RENDERDIST'
        },
        .sun = (struct light_t) {
            .type = LIGHT_DIRECTIONAL,
            .enabled = 1,
            .color = (Color){ 240, 150, 80, 255 },
            .position = (Vector3){ 0, 1, 0 },
            .strength = 0.4,
            .index = SUN_LIGHT_ID
        }
    };
    Material* comfy_material = &gst->terrain.biome_materials[BIOMEID_COMFY];
    *comfy_material = LoadMaterialDefault();
    comfy_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[TERRAIN_TEXID];


    // HAZY BIOME
    struct biome_t* hazy_biome = &gst->terrain.biomedata[BIOMEID_HAZY];
    *hazy_biome = (struct biome_t) {
        .id = BIOMEID_HAZY,
        .fog = (struct fog_t) {
            .mode = FOG_MODE_RENDERDIST,
            .color_top    = (Color){ 25, 25, 25, 255 },
            .color_bottom = (Color){ 25, 25, 25, 255 },
            .density = 0
        },
        .sun = (struct light_t) {
            .type = LIGHT_DIRECTIONAL,
            .enabled = 1,
            .color = (Color){ 200, 160, 100, 255 },
            .position = (Vector3){ 0, 1, 0 },
            .strength = 0.3,
            .index = SUN_LIGHT_ID
        }

    };
    Material* hazy_material = &gst->terrain.biome_materials[BIOMEID_HAZY];
    *hazy_material = LoadMaterialDefault();
    hazy_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[HAZYBIOME_GROUND_TEXID];


    // EVIL BIOME
    struct biome_t* evil_biome = &gst->terrain.biomedata[BIOMEID_EVIL];
    *evil_biome = (struct biome_t) {
        .id = BIOMEID_EVIL,
        .fog = (struct fog_t) {
            .mode = FOG_MODE_RENDERDIST,
            .color_top    = (Color){ 10, 0, 1, 255 },
            .color_bottom = (Color){ 10, 0, 1, 255 },
            .density = 0
        },
        .sun = (struct light_t) {
            .type = LIGHT_DIRECTIONAL,
            .enabled = 1,
            .color = (Color){ 250, 50, 15, 255 },
            .position = (Vector3){ 0, 1, 0 },
            .strength = 0.5,
            .index = SUN_LIGHT_ID
        }
    };
    
    Material* evil_material = &gst->terrain.biome_materials[BIOMEID_EVIL];
    *evil_material = LoadMaterialDefault();
    evil_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[EVILBIOME_GROUND_TEXID];


    gst->terrain.biomeshift_area = 200.0;

    printf("Biome setup done.\n");
}

void setup_biome_ylevels(struct state_t* gst) {
    float biome_div = gst->terrain.highest_point / 3.0;

    gst->terrain.biome_ylevels[BIOMEID_COMFY] = (Vector2) {
        gst->terrain.highest_point,
        biome_div*2
    };

    gst->terrain.biome_ylevels[BIOMEID_HAZY] = (Vector2) {
        biome_div*2,
        biome_div
    };

    gst->terrain.biome_ylevels[BIOMEID_EVIL] = (Vector2) {
        biome_div,
        0
    };
}

void change_to_biome(struct state_t* gst, int biome_id) {

    struct biome_t* biomedata = &gst->terrain.biomedata[biome_id];
    gst->player.current_biome = biomedata;

    gst->fog = biomedata->fog;
    set_fog_settings(gst, &gst->fog);

    gst->sun = biomedata->sun;
    set_light(gst, &gst->sun, LIGHTS_UBO);

    printf("'%s': \"%s\"\n", __func__, get_biome_name_by_id(biome_id));
}


int get_biomeid_by_ylevel(struct state_t* gst, float y) {
    int biomeid = 0;

    for(int i = 0; i < MAX_BIOME_TYPES; i++) {
        Vector2 biome_level = gst->terrain.biome_ylevels[i]; 
        if(y < biome_level.x && y > biome_level.y) {
            biomeid = i;
            break;
        }
    }

    return biomeid;
}

const char* get_biome_name_by_id(size_t biomeid) {
    const char* ptr = NULL;
    const char* names[MAX_BIOME_TYPES] = {
        "Comfy",
        "Hazy",
        "Evil"
    };

    if(biomeid >= MAX_BIOME_TYPES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'biomeid' out of bounds!\033[0m\n",
                __func__);
        goto error;
    }

    ptr = names[biomeid];

error:
    return ptr;
}

void update_biome_envblend(struct state_t* gst) {
    int biomeshift_id = playerin_biomeshift_area(gst, &gst->player);

    if(biomeshift_id >= 0) {
        Vector2 biome_ylevel = gst->terrain.biome_ylevels[biomeshift_id];
        const float py = gst->player.position.y;
        const float shiftarea = gst->terrain.biomeshift_area;

        float T = map(py, biome_ylevel.y, biome_ylevel.y+shiftarea, 0.0, 1.0);
        T = CLAMP(T, 0.0, 1.0);

        //printf("%f | ID:%i, T:%f\n", GetTime(), biomeshift_id, T);

        // Fog settings dont need to be updated if player Y position has not changed.
        if(FloatEquals(py, gst->player.prev_position.y)) {
            return;
        }
       
        
        if(biomeshift_id == 0) {
            // Blend BIOMEID_COMFY <-> BIOMEID_HAZY
            struct biome_t* comfy_biome = &gst->terrain.biomedata[BIOMEID_COMFY];
            struct biome_t* hazy_biome  = &gst->terrain.biomedata[BIOMEID_HAZY];
        
            // This function calls 'set_fog_settings()'.
            fog_blend(gst, &gst->fog, T, &hazy_biome->fog, &comfy_biome->fog);
            
            // Blend sun.
            gst->sun.color = ColorLerp(hazy_biome->sun.color, comfy_biome->sun.color, T);
            set_light(gst, &gst->sun, LIGHTS_UBO);
        }
        else
        if(biomeshift_id == 1) {
            // Blend BIOMEID_HAZY <-> BIOMEID_EVIL
        
            struct biome_t* hazy_biome = &gst->terrain.biomedata[BIOMEID_HAZY];
            struct biome_t* evil_biome = &gst->terrain.biomedata[BIOMEID_EVIL];
            
            fog_blend(gst, &gst->fog, T, &evil_biome->fog, &hazy_biome->fog);
            
            // Blend sun.
            gst->sun.color = ColorLerp(evil_biome->sun.color, hazy_biome->sun.color, T);
            set_light(gst, &gst->sun, LIGHTS_UBO);
        }
    }
}



