#version 330 core
out vec4 outColor;

in vec2 texCoords;

uniform sampler2D noteTexture;
uniform float amount;

void main()
{
    outColor = vec4(vec3(texture(noteTexture, texCoords)), amount);
}
