#version 330 core
out vec4 outColor;

uniform float amount;
uniform vec3 color;

void main()
{
    outColor = vec4(color, amount);
}
