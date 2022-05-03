#version 330 core 

in vec3 fragmentPos;

out vec4 outColor;

uniform float time;

void main()
{
    outColor = vec4(cos(time), sin(time), sin(time), 1.0);
}
