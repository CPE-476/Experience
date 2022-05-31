#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

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

in float Height;
in vec3 Position;
in vec3 Normal;

uniform vec3 viewPos;

uniform float maxFogDistance;
uniform float minFogDistance;
uniform vec4 fogColor;

uniform vec3 bottom;
uniform vec3 top;
uniform vec3 dirt;

uniform float threshold;

void main()
{
    float distanceToCamera = length(Position - viewPos);
    float fogFactor = (maxFogDistance - distanceToCamera) / (maxFogDistance - minFogDistance);
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    vec3 viewDir = normalize(viewPos - Position);

    // shift and scale the height in to a grayscale value
    float h = (Height+7.8) / 10;
    vec3 amount;

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

    FragColor = mix(fogColor, vec4(DirLightColor + PointLightColor, 1.0), fogFactor);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > threshold)
    {
        BrightColor = vec4(FragColor.rgb, 1.0);
    }
    else
    {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    } 
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

