

struct fog_ub_t {
    vec4 density;
    vec4 color_top;
    vec4 color_bottom;
};

layout (std140, binding = 4) uniform fog_ub {
    fog_ub_t fog;
};

float _lerp(float t, float min, float max) {
    return min + t * (max - min);
}

float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

// To blend sky and fog color together at correct position
// use camera position Y and the fragment position Y to get ylevel for fog.
// NOTE: this has a problem with very wide objects on xz axis. But will work for now.

vec3 get_horizon_color(float y) {
    const float max_y  = -1000.0; // Negative value goes up in world.
    const float blevel =  200.0;  // This value controls where bottom color starts.
    const float change =  2.0;    // How fast the color changes from 'color_top' to 'color_bottom'?
    y = map(y, max_y, blevel, 1.0, 0.0);
    y = smoothstep(0.0, 1.0, y*change);
    vec3 res = mix(fog.color_top.rgb, fog.color_bottom.rgb, y);
    return res*0.8;
}

vec3 get_fog(vec3 current, float dist, float ylevel) {
    float f = 1.0/exp((dist*fog.density.x)*(dist*fog.density.x));
    f = clamp(f, 0.0, 1.0);
    vec3 c = get_horizon_color(ylevel);
    c = clamp(c, vec3(0.0), vec3(1.0));
    // Try to break noticeable banding with noise.
    float noise = fract(cos(dot(vec2(-ylevel*200, fract(dist)+ylevel*200), vec2(52.621,67.1262)))*72823.53)/80.0;
    return mix(c, current, f+noise);
};

#define _YLEVEL (fragPosition.y - u_campos.y)

