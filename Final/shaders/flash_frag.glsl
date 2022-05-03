#version 330 core 

out vec4 outColor;

in vec3 fragmentPos;

uniform float time;

void main()
{
    outColor = vec4(0.5, 0.5, 0.5, 1.0);
}
