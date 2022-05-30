#version 330 core 

out vec4 outColor;

uniform vec3 Color;

void main()
{
    outColor = vec4(Color, 1.0f);
}
