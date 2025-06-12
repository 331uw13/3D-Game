


struct biome_info_t {
    vec4 sun_color;
};


layout (std140, binding = 3) uniform biome_ub {
    biome_info_t biome;
};




