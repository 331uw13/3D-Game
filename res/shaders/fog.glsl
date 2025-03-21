

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

/*
#define FOG_DENSITY 0.00082
#define FOG_COLOR_FAR vec3(0.15, 0.25, 0.3)
#define FOG_COLOR_NEAR vec3(0.1, 0.1, 0.1)


vec3 get_fog(vec3 current, float dist) {
    float f = 1.0/exp((dist*FOG_DENSITY)*(dist*FOG_DENSITY));
    f = clamp(f, 0.0, 1.0);
    vec3 c = mix(FOG_COLOR_FAR, FOG_COLOR_NEAR, f*0.5);
    return mix(c, current, f);
}
*/

