#version 330 core

layout (location = 0) in vec3 attribVertexPosition;
layout (location = 1) in vec3 attribColor;
layout (location = 2) in vec2 attribTexCoord;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(attribVertexPosition, 1.0);
    texCoords = attribTexCoord;
}
