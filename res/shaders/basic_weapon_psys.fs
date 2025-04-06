#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;
uniform vec3 viewPos;


uniform vec4 psystem_color;
uniform float time;



#include "res/shaders/voronoi.glsl"



void main()
{
    float v = voronoi3d(fragPosition*0.65).x;
    vec3 col = fragColor.xyz;

    v *= v;

    finalColor = vec4(col*1.25, v*0.5+0.5);
}
