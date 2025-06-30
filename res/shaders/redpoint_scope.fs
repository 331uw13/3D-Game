#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;


// Input uniform values
uniform sampler2D texture0;
uniform vec2 u_screen_size;

// Output fragment color
out vec4 finalColor;



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}


#define PI 3.14159

void main()
{

    float res = 1.0/(length(fragTexCoord.xy-0.5)*60);
    res = pow(res, 1.35);

    finalColor = vec4(1.0, 0.2, 0.5, clamp(res, 0, 1));
}
