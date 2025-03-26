#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

in vec3 fragViewPos;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 player_gun_color;

// Output fragment color
out vec4 finalColor;



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}



void main()
{

    vec3 center = (1.0-vec3(2.5*length(fragTexCoord-0.5))) * 0.3;
    finalColor = texture(texture0, fragTexCoord)*vec4(player_gun_color.rgb + center, 0.5);

}
