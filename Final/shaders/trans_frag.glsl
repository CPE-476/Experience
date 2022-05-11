#version 330 core
out vec4 outColor;

uniform float amount;

void main()
{
    outColor = vec4(1.0, 1.0, 1.0, amount);
}
