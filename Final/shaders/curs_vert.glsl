#version 330 core
layout(location = 0) in vec3 VertexPosition;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(VertexPosition, 1.0);
}
