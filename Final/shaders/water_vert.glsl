#version  330 core
layout(location = 0) in vec3 VertexPosition;

out vec2 UV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(VertexPosition, 1.0);
    //gl_Position = Projection * View * Model * vec4(VertexPosition, 1.0);
    //gl_Position = vec4(VertexPosition, 1.0);
    UV = VertexPosition.xz + vec2(0.5, 0.5);

}