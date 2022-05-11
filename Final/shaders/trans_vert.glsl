
#version 330 core

layout (location = 0) in vec3 attribVertexPosition;
layout (location = 1) in vec3 attribColor;

void main()
{
    gl_Position = vec4(attribVertexPosition, 1.0);
}
