#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

in vec3 fragViewPos;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}

#define PI 3.14159


void main()
{

    vec2 uv = fragTexCoord;
    
    // Coordinate center.
    float uvl = length(uv-0.5);
    uvl *= uvl;

    // Take sin of current pixel and multiply with pi so its in the center.
    // raise it to power of how much distance is to center.
    float v = pow(sin(uv.x*PI), uvl*5000.0) * 0.8;
    float h = pow(sin(uv.y*PI), uvl*5000.0) * 0.8;

    float inv_uvl = 1.0-uvl*5.0; // 5 is the radius.
    float eff = (v+h) * inv_uvl;

    finalColor = vec4(vec3(eff)*fragColor.rgb, eff);
//    vec3 center = (1.0-vec3(2.5*length(fragTexCoord-0.5))) * 0.3;
//    finalColor = texture(texture0, fragTexCoord)*vec4(fragColor.rgb + center, 0.5);

}
