#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;


out vec4 finalColor;
uniform vec3 viewPos;

uniform vec3 psystem_color;

void main()
{
    vec3 col = psystem_color;


    finalColor = vec4(col, 0.8);

}


