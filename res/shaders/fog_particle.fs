
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform float time;

out vec4 finalColor;
//uniform vec3 viewPos;

in vec3 fragViewPos;


#include "res/shaders/fog.glsl"

void main()
{
    vec3 col;
   
    col = vec3(0.08, 0.1, 0.1);
    float dist = length(fragViewPos - fragPosition);
    col = get_fog(col, dist);

    finalColor = vec4(col, 1.0);
}
