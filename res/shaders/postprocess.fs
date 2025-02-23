#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;
uniform float health; // normalized
uniform vec2 screen_size;

// Output fragment color
out vec4 finalColor;

#define Pi 3.14159
#define Pi2 (Pi*2)

float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}

void main()
{

    float gamma = 0.8;


    vec2 size = screen_size * 0.35;
    vec2 sf = 1.0/(size * 2.0);


    vec3 texcolor = texture(texture0, fragTexCoord).rgb;
    finalColor = vec4(texcolor.rgb, 1.0);


    // Blur effect.
    const int r = 3;
    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {

            vec2 c = clamp(fragTexCoord+vec2(x,y)*sf, vec2(0.0), vec2(0.9999));
            texcolor = mix(texcolor,
                    texture(texture0, c).rgb,
                    0.2);


        }
    }

    finalColor = vec4(texcolor, 1.0);
    finalColor = pow(finalColor, vec4(1.0/gamma));
}
