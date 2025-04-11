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

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragViewPos;
out float time;

in mat4 instanceTransform;
layout(location = 8) in vec4 instanceColor;

uniform float u_time;
uniform float u_wind_strength;
uniform vec3  u_wind_dir;

// This is a shader that can be set to any foliage that has wind properties.

#include "res/shaders/voronoi.glsl"




void main()
{

    time = u_time;
    vec3 vertpos = vertexPosition;

    float wind_str = (u_wind_strength / 600.0)*2.0;
    float wind_time = u_time * wind_str;
    float noise_x = voronoi3d(vertpos*0.05 + (u_wind_dir*(wind_time))).x;

    vertpos.x += noise_x * 3.0 + sin(wind_time)*0.65;

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
