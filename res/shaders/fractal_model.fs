
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

in float time;

uniform sampler2D texture0;
uniform vec3 u_campos;

out vec4 finalColor;


#include "res/shaders/voronoi.glsl"
#include "res/shaders/light.glsl"
#include "res/shaders/shadow.glsl"
#include "res/shaders/fog.glsl"


void main()
{
    finalColor = vec4(fragColor.rgb, 1.0);
}
