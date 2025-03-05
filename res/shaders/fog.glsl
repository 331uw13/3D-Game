
#define FOG_DENSITY 0.003
#define FOG_COLOR_FAR vec3(0.3, 0.15, 0.15)
#define FOG_COLOR_NEAR vec3(0.35, 0.1, 0.9)

vec3 get_fog(vec3 current, float dist) {
    float f = 1.0/exp((dist*FOG_DENSITY)*(dist*FOG_DENSITY));
    f = clamp(f, 0.0, 1.0);
    vec3 c = mix(FOG_COLOR_FAR, FOG_COLOR_NEAR, f*0.5);
    return mix(c, current, f);
}

