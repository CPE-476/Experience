#version 330 core

layout (location = 0) in vec3 attribVertexPosition;
layout (location = 1) in vec2 attribTexCoord;

uniform mat4 transform;

out vec2 texCoords;

void main()
{
    gl_Position = transform * vec4(attribVertexPosition, 1.0);
    texCoords = attribTexCoord;
}
