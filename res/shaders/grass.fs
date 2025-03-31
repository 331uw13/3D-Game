#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

in vec3 fragViewPos;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 u_gunfx_color;

// Output fragment color
out vec4 finalColor;



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}



void main()
{

    finalColor = vec4(0.1, 0.5, 0.1, 1.0);
}
