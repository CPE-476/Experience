#version 330 core 
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shine;
};

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

uniform DirLight dirLight;
#define MAX_LIGHTS 128
uniform PointLight pointLights[MAX_LIGHTS];
uniform int size;

uniform Material material;
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

    //outColor = mix(fogColor, vec4(PointLightColor + DirLightColor, 1.0), fogFactor);
    outColor = vec4(0.5f, 0.5f, 1.0f, 0.5f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(halfwayDir, reflectDir), 0.0), material.shine);
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(halfwayDir, reflectDir), 0.0), material.shine);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}
