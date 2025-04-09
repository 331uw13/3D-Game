#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;


// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int u_reflect_include;
// Output fragment color
out vec4 finalColor;


void main()
{
    finalColor = vec4(float(u_reflect_include));
}
