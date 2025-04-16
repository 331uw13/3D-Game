
#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 u_grass_mvp;


// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;


void main()
{
    fragPosition = vertexPosition;

    // Calculate final vertex position
    gl_Position = u_grass_mvp * vec4(vertexPosition, 1.0);    
}
