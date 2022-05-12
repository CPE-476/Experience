#version 330 core 

in vec2 UV;

out vec4 outColor;

uniform sampler2D myTex;

void main()
{
    outColor = vec4(0.2, 0.2, 1.0, 0.4);
    outColor = texture(myTex, UV);
}