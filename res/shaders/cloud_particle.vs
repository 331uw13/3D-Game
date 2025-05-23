#version 430


// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;


// Input uniform values
uniform mat4 mvp;
uniform mat4 matNormal;
uniform vec3 viewPos;
uniform float u_time;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragViewPos;

in mat4 instanceTransform;
layout(location = 8) in vec4 instanceColor;

#include "res/shaders/voronoi.glsl"

void main()
{
    vec3 vertpos = vertexPosition;

    vec3 scroll = vec3(0, u_time, 0);
    float noise_y = voronoi3d(vertpos*0.2 + scroll).x;
    float noise_x = voronoi3d(vertpos*0.05 + scroll*0.5).x * voronoi3d(vertpos*0.035+scroll*0.35).x;

    vertpos.y += noise_y*2;
    vertpos.xz += noise_x*3.0;


    // Send vertex attributes to fragment shader
    fragPosition = vec3(instanceTransform * vec4(vertpos, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = instanceColor;
    

    vec4 transfpos = instanceTransform * vec4(vertpos,1.0);
    fragViewPos = viewPos;
    
    vec4 norm = vec4(vertexNormal, 0.0);
    fragNormal = vec3(instanceTransform * norm);

    //mat3 normalMatrix = transpose(inverse(mat3(instanceTransform)));
    //fragNormal = normalMatrix * vertexNormal;

    // Calculate final vertex position
    gl_Position = mvp * instanceTransform * vec4(vertpos, 1.0);
}
