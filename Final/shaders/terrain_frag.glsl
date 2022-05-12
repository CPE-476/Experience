#version 330 core

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 amount);

out vec4 outColor;

in float Height;
in vec3 Position;
in vec3 Normal;

uniform vec3 viewPos;

uniform vec3 bottom;
uniform vec3 top;
uniform vec3 dirt;

void main()
{
    vec3 viewDir = normalize(viewPos - Position);

    //bottom = vec3(1, 0, 0);
    //top = vec3(0, 1, 0);  //uncomment for RGB debugger
    //dirt = vec3(0, 0, 1); 
    float h = (Height+7.8) / 10;  // shift and scale the height in to a grayscale value
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

    //vec3 amount = mix(bottom, top, h);
    
    outColor = vec4(DirLightColor, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 amount)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * amount;
    vec3 diffuse = light.diffuse * diff * amount;

    return (ambient + diffuse);
}
