#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gDepth;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;


void main() {
    gPosition = fragPosition;
    gNormal = normalize(fragNormal);
    gAlbedoSpec = fragColor;


    float near = 0.01;
    float far = 300.0;
    float ndc = gl_FragCoord.z * 2.0 - 1.0;
    float ld = (2.0 * near * far) / (far + near - ndc * (far - near));
    gDepth = vec4(ld)/far;
}
