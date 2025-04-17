
#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 u_viewproj;


// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out mat4 viewproj;


void main()
{
    fragPosition = vertexPosition;
    viewproj = u_viewproj;
    gl_Position = u_viewproj * vec4(vertexPosition, 1.0);    
}
