
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in float vf_time;

uniform sampler2D texture0;
uniform vec3 u_campos;

out vec4 finalColor;
uniform vec4 psystem_color;
//uniform vec3 viewPos;



#include "res/shaders/fog.glsl"

void main()
{
    vec3 col;
   
    col = psystem_color.rgb;
    float dist = length(u_campos - fragPosition);
    col = get_fog(col, dist, _YLEVEL);

    finalColor = vec4(col, psystem_color.w);
}
