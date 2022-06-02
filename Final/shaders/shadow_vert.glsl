#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec2 TexCoords;
out float Height;
out vec3 Position;
out vec3 Normal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    float isTerrain;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;
uniform bool isT;

void main()
{
    Height = aPos.y;
    Normal = aNormal;
    Position = aPos;
    if(!isT)
    {
        vs_out.isTerrain = 0;
        vs_out.FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
        vs_out.Normal = transpose(inverse(mat3(instanceMatrix))) * aNormal;
        vs_out.TexCoords = aTexCoords;
        vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    }
    else
    {
        vs_out.isTerrain = 1;
        vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
        vs_out.TexCoords = aTexCoords;
        vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
}
