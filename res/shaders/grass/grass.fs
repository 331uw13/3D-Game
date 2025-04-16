#version 430

in vec3 fragPosition;


out vec4 finalColor;



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}



void main()
{
    finalColor = vec4(0.1, 0.25, 0.2, 1.0);
}
