#version 430

in vec3  fragPosition;
in float grassblade_base_y;
in vec3  force_vector;

out vec4 finalColor;

vec3 fragNormal = vec3(0);

uniform vec3 u_campos;


#include "res/shaders/light.glsl"
#include "res/shaders/fog.glsl"


float lerp(float t, float min, float max) {
    return min + t * (max - min);
}

vec3 color_lerp(float t, vec3 a, vec3 b) {
    return vec3(
            lerp(t, a.r, b.r),
            lerp(t, a.g, b.g),
            lerp(t, a.b, b.b)
            );
}


float get_noise(vec2 noisepos) {
    return fract(cos(dot(noisepos, vec2(52.621,67.1262)))*72823.53)/10.0;
}

void main()
{

    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    //vec3 color = vec3(0.0, 0.3, 0.3);
    //finalColor = vec4(color, 1.0);

    vec3 color = vec3(0);

    // Fake normal.
    vec3 dx = dFdx(fragPosition);
    vec3 dy = dFdy(fragPosition);
    fragNormal = normalize(cross(dx, dy));


    vec3 view_dir = normalize(u_campos - fragPosition);
    compute_lights(view_dir);

    // Change color towards the grass blade tip.
    // TODO: Support more colors than two.
    vec3 grass_color 
        = color_lerp(
                (fragPosition.y - grassblade_base_y) / 10.0,
                vec3(0.02, 0.07, 0.0),
                vec3(0.2, 0.12, 0.0)
                );

    color = g_lightcolor*0.258 + grass_color;

    // Gamma correction.
    color = pow(color, vec3(1.0/0.54));

    float dist = length(u_campos - fragPosition);
    color = get_fog(color, dist, _YLEVEL);

    finalColor.rgb = color;
}
