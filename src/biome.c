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
        .sun = (Color){ 0xC4, 0xAF, 0x81, 0xFF }
        
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
            .color_top    = (Color){ 15, 15, 15, 255 },
            .color_bottom = (Color){ 25, 25, 25, 255 },
            .density = 0
        },
        .sun = (Color){ 0x81, 0xC4, 0xB0, 0xFF }

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
            .color_top    = (Color){ 5, 0, 1, 255 },
            .color_bottom = (Color){ 10, 0, 1, 255 },
            .density = 0
        },
        .sun = (Color){ 0xC4, 0x81, 0xA5, 0xFF }
    };
    
    Material* evil_material = &gst->terrain.biome_materials[BIOMEID_EVIL];
    *evil_material = LoadMaterialDefault();
    evil_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[EVILBIOME_GROUND_TEXID];


    gst->terrain.biomeshift_area = 200.0;

    printf("Biome setup done.\n");
}

void setup_biome_areas(struct state_t* gst) {
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

// Set biome data for UBO.
void update_biome_data(struct state_t* gst, struct biome_t* biome) {
    float biome_sun_color[4] = {
        (float)biome->sun.r / 255.0,
        (float)biome->sun.g / 255.0,
        (float)biome->sun.b / 255.0,
        1.0
    };

    glBindBuffer(GL_UNIFORM_BUFFER, gst->ubo[BIOME_UBO]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float)*4, biome_sun_color);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void change_to_biome(struct state_t* gst, int biome_id) {

    struct biome_t* biome = &gst->terrain.biomedata[biome_id];
    gst->player.current_biome = biome;

    gst->fog = biome->fog;
    set_fog_settings(gst, &gst->fog);

    update_biome_data(gst, biome);
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

        // Fog settings dont need to be updated if player Y position has not changed.
        if(FloatEquals(py, gst->player.prev_position.y)) {
            return;
        }
       

        struct biome_t* biome_A = NULL;
        struct biome_t* biome_B = NULL;
        
        if(biomeshift_id == 0) { // Blend BIOMEID_COMFY <-> BIOMEID_HAZY
            biome_A = &gst->terrain.biomedata[BIOMEID_HAZY];
            biome_B = &gst->terrain.biomedata[BIOMEID_COMFY];
        }
        else
        if(biomeshift_id == 1) { // Blend BIOMEID_HAZY <-> BIOMEID_EVIL
            biome_A = &gst->terrain.biomedata[BIOMEID_EVIL];
            biome_B = &gst->terrain.biomedata[BIOMEID_HAZY];
        }

        if(biome_A && biome_B) {
            struct biome_t liminal_biome = (struct biome_t) {
                .sun = ColorLerp(biome_A->sun, biome_B->sun, T)
            };

            fog_blend(gst, &gst->fog, T, &biome_A->fog, &biome_B->fog);
            update_biome_data(gst, &liminal_biome);
        }
    }
}



