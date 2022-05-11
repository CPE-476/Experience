#version  330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoordinates;
layout(location = 3) in mat4 instanceMatrix;


out vec3 fragmentPos;
out vec3 normal;
out vec2 texCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    fragmentPos = vec3(instanceMatrix * vec4(vertexPos, 1.0));
    normal = mat3(transpose(inverse(instanceMatrix))) * vertexNormal;

    texCoords = textureCoordinates;
    gl_Position = projection * view * vec4(fragmentPos, 1.0);
}
