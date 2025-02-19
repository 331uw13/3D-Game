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

#define     MAX_LIGHTS              4
#define     MAX_PROJECTILE_LIGHTS   64
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1


struct MaterialProperty {
    vec3 color;
    int useSampler;
    sampler2D sampler;
};

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform Light projlights[MAX_PROJECTILE_LIGHTS];

uniform vec4 ambient;
uniform vec3 viewPos;
uniform float fogDensity;

float lerp(float t, float min, float max) {
    return min + t * (max - min);
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL) { 
                light = -normalize(lights[i].target - lights[i].position);
            }
            if (lights[i].type == LIGHT_POINT) { 
                light = normalize(lights[i].position - fragPosition);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL;

            float specCo = 0.0;
            if (NdotL > 0.0) { 
                specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // Shine: 16.0
            }
            specular += specCo;
        }
    }


    // calculate projectile lights

    for (int i = 0; i < MAX_PROJECTILE_LIGHTS; i++)
    {
        if (projlights[i].enabled == 1)
        {
            vec3 lightdir = normalize(projlights[i].position - fragPosition);
           

            // fix the light radius.
            float lradius = 2.5;
            float dist = distance(projlights[i].position, fragPosition) / lradius;
            dist = 1.0/dist;

            float NdotL = max(dot(normal, lightdir), 0.0);
            lightDot += (projlights[i].color.rgb * NdotL) * dist;

            float specCo = 0.0;
            if(NdotL > 0.0) {
                specCo = pow(max(0.0, dot(viewD, reflect(-lightdir, normal))), 36.0);
            }


            specular += (dist * projlights[i].color.rgb) * (specCo * specCo);
        }
    }


    finalColor = (texelColor*((colDiffuse + vec4(specular,1))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0);

    // Gamma correction
    
    float dist = length(viewPos - fragPosition);

    //const vec4 fogColor = vec4(0.0, 0.5, 0.5, 1.0);

    /*
    // interesting effect..
    float lr = lerp((fragPosition.y*0.002)-0.3, 1.0, 0.0);
    lr = pow(lr, 6.0);
    lr = max(lr, 0.1);
    */
    vec4 fogColor = vec4(0.05, 0.1, 0.1, 1.0); //* lr;

    fogColor.w = 1.0;

    float fogFactor = 1.0/exp((dist*fogDensity)*(dist*fogDensity));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    finalColor = mix(fogColor, finalColor, fogFactor);
    
    // very steep exponental for the alpha layer.
    finalColor.w = pow(fogFactor * fogFactor * fogFactor, 5.0);
    finalColor = pow(finalColor, vec4(1.0/2.2));
}


