#version 330 core
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_LIGHTS 128
uniform PointLight pointLights[MAX_LIGHTS];
uniform int size;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 amount);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 amount);

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    float isTerrain;
} fs_in;

in float Height;
in vec3 Position;
in vec3 Normal;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float maxFogDistance;
uniform float minFogDistance;
uniform vec4 fogColor;

uniform vec3 bottom;
uniform vec3 top;
uniform vec3 dirt;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0; 

    return shadow;
}

void main()
{ 
    float distanceToCamera = length(Position - viewPos);
    float fogFactor = (maxFogDistance - distanceToCamera) / (maxFogDistance - minFogDistance);
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    vec3 amount;
    float h;          
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    if(fs_in.isTerrain == 1)
    {
        h = (Height+7.8) / 10;
        if (h < 0.5)
        {
            amount = (bottom * h * 2.0) +  dirt * (0.5 - h) * 2.0;
        }
        else
        {
            amount = top * (h - 0.5) * 2.0 + bottom * (1.0 - h) * 2.0;
        }

        vec3 DirLightColor = CalcDirLight(dirLight, Normal, viewDir, amount);

        vec3 PointLightColor = vec3(0.0);
        for(int i = 0; i < size; ++i)
        {
            PointLightColor += CalcPointLight(pointLights[i], Normal, Position, viewDir, amount);
        }

        color = mix(fogColor, vec4(DirLightColor + PointLightColor, 1.0), fogFactor).rgb;
    }
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 amount)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * amount;
    vec3 diffuse = light.diffuse * diff * amount;

    return (ambient + diffuse);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 amount)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 diffuse = light.diffuse * diff * amount;
    diffuse *= attenuation;

    return diffuse;
}
