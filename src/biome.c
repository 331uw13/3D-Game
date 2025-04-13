#include <stdio.h>
#include <stdlib.h>

#include "state/state.h"
#include "biome.h"



void setup_biomes(struct state_t* gst) {


    // COMFY BIOME
    struct biome_t* comfy_biome = &gst->terrain.biomedata[BIOMEID_COMFY];
    *comfy_biome = (struct biome_t) {
        .id = BIOMEID_COMFY,
        .sun_color = (Color){ 255, 140, 30, 255 },
        .fog = (struct fog_t) {
            .mode = FOG_MODE_RENDERDIST,
            .color_top    = (Color){ 16, 0, 25, 255 },
            .color_bottom = (Color){ 39, 0, 37, 255 },
            .density = 0 // Density is ignored when fog mode is 'FOG_MODE_RENDERDIST'
        },
    };
    Material* comfy_material = &gst->terrain.biome_materials[BIOMEID_COMFY];
    *comfy_material = LoadMaterialDefault();
    comfy_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[TERRAIN_TEXID];


    // HAZY BIOME
    struct biome_t* hazy_biome = &gst->terrain.biomedata[BIOMEID_HAZY];
    *hazy_biome = (struct biome_t) {
        .id = BIOMEID_HAZY,
        .sun_color = (Color){ 50, 140, 255, 255 },
        .fog = (struct fog_t) {
            .mode = FOG_MODE_RENDERDIST,
            .color_top    = (Color){ 30, 30, 30, 255 },
            .color_bottom = (Color){ 15, 15, 15, 255 },
            .density = 0
        },
    };
    Material* hazy_material = &gst->terrain.biome_materials[BIOMEID_HAZY];
    *hazy_material = LoadMaterialDefault();
    hazy_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[HAZYBIOME_GROUND_TEXID];


    // EVIL BIOME
    struct biome_t* evil_biome = &gst->terrain.biomedata[BIOMEID_EVIL];
    *evil_biome = (struct biome_t) {
        .id = BIOMEID_EVIL,
        .sun_color = (Color){ 200, 10, 25, 255 },
        .fog = (struct fog_t) {
            .mode = FOG_MODE_RENDERDIST,
            .color_top    = (Color){ 30, 30, 30, 255 },
            .color_bottom = (Color){ 15, 15, 15, 255 },
            .density = 0
        },
    };
    
    Material* evil_material = &gst->terrain.biome_materials[BIOMEID_EVIL];
    *evil_material = LoadMaterialDefault();
    evil_material->maps[MATERIAL_MAP_DIFFUSE].texture 
        = gst->textures[EVILBIOME_GROUND_TEXID];


    // Set biome y levels. where they start and end.
    // TODO: Make this scale automatically to terrain points.


    gst->terrain.biome_ylevels[BIOMEID_COMFY] = (Vector2) {
        gst->terrain.highest_point+1000,
        gst->terrain.highest_point-300,
    };

    gst->terrain.biome_ylevels[BIOMEID_HAZY] = (Vector2) {
        gst->terrain.highest_point-300,
        gst->terrain.highest_point-5000,
    };

    gst->terrain.biome_ylevels[BIOMEID_EVIL] = (Vector2) {
        gst->terrain.highest_point-5000,
        gst->terrain.lowest_point,
    };

    printf("Biome setup done.\n");
}



