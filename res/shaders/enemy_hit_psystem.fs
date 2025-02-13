#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;


out vec4 finalColor;
uniform vec3 viewPos;
uniform vec3 gravity_point;
uniform float time;


#define PI 3.14159
#define PI2 (PI*2)


void main()
{


    finalColor = vec4(vec3(0.01, 0.9, 0.9), 0.455);

}


