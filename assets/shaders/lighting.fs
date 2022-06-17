#version 300 es

// Input vertex attributes (from vertex shader)
in lowp vec3 fragPosition;
in lowp vec2 fragTexCoord;
in lowp vec4 fragColor;
in lowp vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform lowp vec4 colDiffuse;

// Output fragment color
out lowp vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct MaterialProperty {
    lowp vec3 color;
    int useSampler;
    sampler2D sampler;
};

struct Light {
    int enabled;
    int type;
    lowp vec3 position;
    lowp vec3 target;
    lowp vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform lowp vec4 ambient;
uniform lowp vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    lowp vec4 texelColor = fragColor;
    lowp vec3 lightDot = vec3(0.0);
    lowp vec3 normal = normalize(fragNormal);
    lowp vec3 viewD = normalize(viewPos - fragPosition);
    lowp vec3 specular = vec3(0.0);

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            lowp vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
            }

            lowp float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL;

            lowp float specCo = 0.0;
            if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine
            specular += specCo;
        }
    }

    finalColor = (texelColor*((colDiffuse + vec4(specular, 1.0))* vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0)*colDiffuse;

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}