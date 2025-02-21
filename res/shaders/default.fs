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

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              (64 + 4)
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1



struct Light {
    int enabled;
    int type;
    vec3 position;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
//uniform Light projlights[MAX_PROJECTILE_LIGHTS];

uniform vec4 ambient;
//uniform vec3 fragViewPos;
uniform float fogDensity;

in vec3 fragViewPos;

float lerp(float t, float min, float max) {
    return min + t * (max - min);
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(fragViewPos - fragPosition);
    vec3 specular = vec3(0.0);


    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            float dist = 1.0;
            vec3 lightdir = vec3(0);
            
            if(lights[i].type == LIGHT_POINT) {
                lightdir = normalize(lights[i].position - fragPosition);
                // fix the light radius.
                float lradius = 2.5;
                dist = distance(lights[i].position, fragPosition) / lradius;
                dist = 1.0/dist;
            }
            else if(lights[i].type == LIGHT_DIRECTIONAL) {
                lightdir = -normalize(-lights[i].position);
            }

            float NdotL = max(dot(normal, lightdir), 0.0);
            lightDot += (lights[i].color.rgb * NdotL) * dist;

            float specCo = 0.0;
            if(NdotL > 0.0) {
                specCo = pow(max(0.0, dot(viewD, reflect(-lightdir, normal))), 36.0);
            }


            specular += (dist * lights[i].color.rgb) * (specCo * specCo);
        }
    }


    finalColor = (texelColor*((colDiffuse + vec4(specular,1))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0);

    // Gamma correction
    
    // very steep exponental for the alpha layer.
    finalColor = pow(finalColor, vec4(1.0/2.2));
    float dist = length(fragViewPos - fragPosition);

    //const vec4 fogColor = vec4(0.0, 0.5, 0.5, 1.0);

    // interesting effect..
    float lr = lerp((fragPosition.y*0.002)-0.3, 1.0, 0.0);
    lr = pow(lr, 10.0);
   
    lr *= 0.01;

    vec4 fogColor = vec4(0.0, 0.1, 0.1, 1.0);


    fogColor.w = 1.0;

    float fogFactor = 1.0/exp((dist*fogDensity)*(dist*fogDensity));
    fogFactor = clamp(fogFactor, 0.0, 1.0);


    vec4 fogColor2 = vec4(0.2, 0.1, 0.6, 1.0);

    fogColor = mix(fogColor, fogColor2, fogFactor/2);


    finalColor = mix(fogColor, finalColor, fogFactor);
}


