#version 330 core

layout(location = 0) in vec4 vertexPos;
out vec2 texCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertexPos.xy, 0, 1);
    texCoords = vertexPos.zw;
}
