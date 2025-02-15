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


    finalColor = vec4(vec3(0.3, 0.9, 0.9), 0.455);

}


