#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;



// NOTE: these 2 values must same as in 'light.h'
#define MAX_PROJECTILE_LIGHTS 512
#define MAX_NORMAL_LIGHTS 4

#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1



struct Light {
    int enabled;
    int type;
    vec3 position;
    vec4 color;
};

uniform Light lights[MAX_NORMAL_LIGHTS];
uniform Light prj_lights[MAX_PROJECTILE_LIGHTS];


uniform vec4 ambient;
uniform float fogDensity;

in vec3 fragViewPos;



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}


vec3 g_lightdot = vec3(0.0);
vec3 g_specular = vec3(0.0);


void compute_light(vec3 light_position, int light_type, vec4 light_color, vec3 viewD, vec3 normal) {
    float dist = 1.0;
    vec3 lightdir = vec3(0);
    
    if(light_type == LIGHT_POINT) {
        lightdir = normalize(light_position - fragPosition);
        // fix the light radius.
        float lradius = 2.5;
        dist = distance(light_position, fragPosition) / lradius;
        dist = 1.0/dist;
    }
    else if(light_type == LIGHT_DIRECTIONAL) {
        lightdir = -normalize(-light_position);
    }

    float NdotL = max(dot(normal, lightdir), 0.0);
    g_lightdot += (light_color.rgb * NdotL) * dist;

    float specCo = 0.0;
    if(NdotL > 0.0) {
        specCo = pow(max(0.0, dot(viewD, reflect(-lightdir, normal))), 36.0);
    }

    if(light_type != LIGHT_DIRECTIONAL) {
    g_specular += (dist * light_color.rgb) * (specCo * specCo);
    }
}




void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(fragViewPos - fragPosition);


    for (int i = 0; i < MAX_NORMAL_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            compute_light(
                    lights[i].position,
                    lights[i].type,
                    lights[i].color,
                    viewD,
                    normal
                    );
        }
    }

    for (int i = 0; i < MAX_PROJECTILE_LIGHTS; i++)
    {
        if (prj_lights[i].enabled == 1)
        {
            compute_light(
                    prj_lights[i].position,
                    prj_lights[i].type,
                    prj_lights[i].color,
                    viewD,
                    normal
                    );
        }
    }


    finalColor = (texelColor * ((colDiffuse + vec4(g_specular, 1.0)) * vec4(g_lightdot,1.0)));
    finalColor += texelColor * (ambient/6.0);

    // Scale colors back to more bright while post processing
    vec3 mapped = finalColor.xyz / (finalColor.xyz + vec3(1.6));
    finalColor.xyz = pow(mapped, vec3(1.0 / 0.6));
   

    float dist = length(fragViewPos - fragPosition);


    // interesting effect..
    float lr = lerp((fragPosition.y*0.002)-0.3, 1.0, 0.0);
    lr = pow(lr, 10.0);
    lr *= 0.01;
    vec4 fogColor = vec4(0.3, 0.15, 0.15, 1.0);

    float fogFactor = 1.0/exp((dist*fogDensity)*(dist*fogDensity));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec4 fogColor2 = vec4(0.35, 0.1, 0.9, 1.0);
    fogColor = mix(fogColor, fogColor2, fogFactor/2);


    finalColor = mix(fogColor, finalColor, fogFactor);
}


