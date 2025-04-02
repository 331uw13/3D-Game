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

// Output fragment color
out vec4 finalColor;

#include "res/shaders/fog.glsl"
#include "res/shaders/light.glsl"
#include "res/shaders/voronoi.glsl"





void main()
{
    vec3 sun_point = fragPosition - u_campos;
    /*
    vec3 sun_point = fragPosition - u_campos;
    vec3 sun_pos = vec3(0.0, 1.0, -1.0) * u_render_dist;
    float sun = length(sun_point- sun_pos) / (u_render_dist*2);
    sun = sun * sun;
    */

    //float y = length((sun_point.y) * step(0.0, sun_point.y)) / (u_render_dist*0.85);
    finalColor.rgb = get_horizon_color(sun_point.y);
    finalColor.w = 1.0;
}
