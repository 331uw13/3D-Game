

struct fog_ub_t {
    vec4 density;
    vec4 color_near;
    vec4 color_far;
};

layout (std140, binding = 4) uniform fog_ub {
    fog_ub_t fog;
};

vec3 get_fog(vec3 current, float dist) {
    float f = 1.0/exp((dist*fog.density.x)*(dist*fog.density.x));
    f = clamp(f, 0.0, 1.0);
    vec3 c = mix(fog.color_far.rgb, fog.color_near.rgb, f*0.5);
    return mix(c*0.6, current, f);
};


