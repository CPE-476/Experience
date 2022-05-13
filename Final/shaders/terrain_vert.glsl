#version 330 core
layout (location = 0) in vec3 attribVertexPosition;
layout (location = 1) in vec3 attribVertexNormal;

out float Height;
out vec3 Position;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = attribVertexNormal;
    Height = attribVertexPosition.y;
    Position = attribVertexPosition;
    gl_Position = projection * view * model * vec4(attribVertexPosition, 1.0);
}
