#version  330 core

layout(location = 0) in vec3 vertexPosition;

out vec3 fragmentPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0f);
}
