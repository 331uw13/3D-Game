
// https://github.com/raysan5/raylib/blob/master/examples/shaders/resources/shaders/glsl330/gbuffer.fs

#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

//uniform sampler2D diffuseTexture;
//uniform sampler2D specularTexture;

void main() {
    gPosition = fragPosition;
    gNormal = normalize(fragNormal);
    gAlbedoSpec = fragColor;
}
