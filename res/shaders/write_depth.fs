
#version 430

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;


out vec4 finalColor;



void main() {

    finalColor.xyz = fragPosition;
    /*
    float near = 0.01;
    float far = 40.0;
    float ndc = gl_FragCoord.z * 2.0 - 1.0;
    
    float noise = fract(cos(dot(vec2(ndc*200, -ndc*200),
                    vec2(52.621,67.1262)))*72823.53)/238.0;
    
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
    finalColor = vec4(vec3(linearDepth) / far + noise, 1.0);
    finalColor.w = 1.0;
    */
}


