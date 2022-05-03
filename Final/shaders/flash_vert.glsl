#version 330 core
layout(location = 0) in vec3 vertexPosition;

out vec3 fragmentPos;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main()
{
    gl_Position = Projection * View * Model * vec4(vertexPosition, 1.0);
}
