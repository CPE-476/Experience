#version 330 core

in vec2 texCoords;
out vec4 outColor;

uniform sampler2D Text;
uniform vec3 TextColor;

void main()
{
    vec4 Sampled = vec4(1.0, 1.0, 1.0, texture(Text, texCoords).r);
    outColor = vec4(TextColor, 1.0) * Sampled;
}
