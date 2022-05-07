#version 330 core 

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

out vec4 outColor;

in vec3 normal;
in vec3 fragmentPos;
in vec2 texCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_opacity1;

uniform int sample_diffuse1 = 0;
uniform int sample_specular1 = 0;
uniform int sample_normal1 = 0;
uniform int sample_height1 = 0;
uniform int sample_opacity1 = 0;

uniform DirLight dirLight;
#define MAX_LIGHTS 128
uniform PointLight pointLights[MAX_LIGHTS];
uniform int size;

uniform float shine;
uniform vec3 viewPos;

uniform float maxFogDistance;
uniform float minFogDistance;
uniform vec4 fogColor;

void main()
{
    float distanceToCamera = length(fragmentPos - viewPos);
    float fogFactor = (maxFogDistance - distanceToCamera) / (maxFogDistance - minFogDistance);
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragmentPos);
    
    vec3 DirLightColor = CalcDirLight(dirLight, norm, viewDir);

    vec3 PointLightColor = vec3(0.0);
    for(int i = 0; i < size; ++i)
    {
        PointLightColor += CalcPointLight(pointLights[i], norm, fragmentPos, viewDir);
    }
    if(sample_opacity1 == 1)
    {
        if(vec3(texture(texture_opacity1, texCoords)).x == 0)
        {
            discard;
        }
    }

    outColor = mix(fogColor, vec4(PointLightColor + DirLightColor, 1.0), fogFactor);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(halfwayDir, reflectDir), 0.0), shine);

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_diffuse1, texCoords));

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(halfwayDir, reflectDir), 0.0), shine);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_diffuse1, texCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}
