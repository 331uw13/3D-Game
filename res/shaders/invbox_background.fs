#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

out vec4 finalColor;

uniform float u_time;
uniform vec2 u_screen_size;
uniform vec4 u_item_rarity_color;


float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

#include "res/shaders/voronoi.glsl"


// https://iquilezles.org/articles/distfunctions2d/
float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}



void main()
{
    vec3 color = u_item_rarity_color.rgb;
    vec2 uv = fragTexCoord - 0.5;
    float dampen = u_item_rarity_color.a;



    //uv.x += sin(u_time*3.52312 + uv.y*10.0)*0.0092;
    //uv.y += cos(u_time*1.25572 + uv.x*10.0)*0.0092;

    float box = abs(sdBox(uv, vec2(0.32))-0.075);
        
    box = pow(0.09/box, 1.0);
    color *= (box/6.0);

    finalColor = vec4(clamp(color, 0, 1) * dampen * 0.8, length(color));
}



