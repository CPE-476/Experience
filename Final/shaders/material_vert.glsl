#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in mat4 instanceMatrix;

out vec3 fragmentPos;
out vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    fragmentPos = vec3(instanceMatrix * vec4(vertexPosition, 1.0));
    normal = mat3(transpose(inverse(model))) * vertexNormal;

    gl_Position = projection * view * vec4(fragmentPos, 1.0);
}
