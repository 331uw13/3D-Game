#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

in vec3 fragViewPos;

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
    vec2 uv = gl_FragCoord.xy / u_screen_size;
    uv = uv * 2.0 - 1.0;
    uv.x *= (u_screen_size.x / u_screen_size.y);
    
    float radius = 1.425; // NOTE: Radius is inverted.
    float uvl = length(uv) * radius;

    uvl = 1.0-pow(uvl, 10.0);
   

    float line_width = 0.0001;
    float line_h = step(sin(uv.y*8.0+PI/2.0), 1.0-line_width); // Horizontal.
    float line_v = step(sin(uv.x*8.0+PI/2.0), 1.0-line_width); // Vertical.

    float res = line_h * line_v * uvl;

    finalColor = vec4(vec3(0), 1.0-res);
}
