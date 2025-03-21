
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
    vec3 col = vec3(0.7, 0.0, 0.96) + (0.2*(cos(fragNormal.z) + 0.5*sin(time - fragPosition.y)*0.5+0.5));
    col = clamp(col, vec3(0.0), vec3(1.0));

    float dist = length(fragViewPos - fragPosition);
    col = get_fog(col, dist);

    finalColor = vec4(col, 0.8);
}
