
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform float time;

out vec4 finalColor;
uniform vec4 psystem_color;
//uniform vec3 viewPos;

in vec3 fragViewPos;


#include "res/shaders/fog.glsl"

void main()
{
    vec3 col;
   
    col = psystem_color.rgb;
    float dist = length(fragViewPos - fragPosition);
    col = get_fog(col, dist);

    finalColor = vec4(col, psystem_color.w);
}
