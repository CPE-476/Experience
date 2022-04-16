#version 330 core

out vec4 outColor;

in vec3 texCoords;
uniform samplerCube cubemap;

void main()
{
    outColor = texture(cubemap, texCoords);
}
