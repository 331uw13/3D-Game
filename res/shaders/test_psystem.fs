#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;


out vec4 finalColor;
uniform vec3 viewPos;


void main()
{

    float r = 0.5+0.5*cos(fragPosition.x * 0.05);
    float g = 0.5+0.5*sin(fragPosition.y * 0.05);
    float b = 0.5+0.5*cos(fragPosition.z * 0.05);

    finalColor = vec4(r,g,b, 0.65);

}
