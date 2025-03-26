
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;
uniform vec3 viewPos;


// TODO: Remove these from all psystem shaders and clean them up.
uniform vec4 psystem_color;
uniform float time;


#include "res/shaders/voronoi.glsl"

void main()
{
    vec3 color = fragColor.rgb;

    float vnoise = voronoi3d(fragPosition*0.1).x;
    float a = fragColor.w * vnoise;

    finalColor = vec4(color, a);
}
