#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;


// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 u_campos;
uniform float u_render_dist;
uniform float u_time;
uniform vec4 u_sun_color;

// Output fragment color
out vec4 finalColor;

#include "res/shaders/fog.glsl"
#include "res/shaders/light.glsl"
#include "res/shaders/voronoi.glsl"




void main()
{

    const float effect_mult = 22.0;

    vec3 sun_point = fragPosition - u_campos;
   
    const float damp = 0.7;
    vec3 sun_pos = vec3(0.0, 1.0, 0.0) * u_render_dist;
    float sun = length(sun_point - sun_pos)/(u_render_dist*2);
    float rad = sun;
    sun = pow(sun, 0.5);

    vec3 scroll = vec3(0.0, -u_time*0.35, 0.0);
    vec3 noisepos = (sun * effect_mult) * (sun_point*0.00035);
    sun += voronoi3d(noisepos + scroll).x*0.2+rad;


    sun = damp-clamp(sun, 0.0, damp);
    
    finalColor.rgb = get_horizon_color(sun_point.y) + (sun*u_sun_color.rgb*(1.0/rad*0.1));
    finalColor.w = 1.0;
}
