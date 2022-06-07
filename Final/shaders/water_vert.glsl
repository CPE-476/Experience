#version  330 core
layout(location = 0) in vec3 VertexPosition;

out vec2 UV;
out vec4 w_color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 color;

void main()
{
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
    //gl_Position = Projection * View * Model * vec4(VertexPosition, 1.0);
    //gl_Position = vec4(VertexPosition, 1.0);
    UV = VertexPosition.xz + vec2(0.5, 0.5);
    w_color = color;

}