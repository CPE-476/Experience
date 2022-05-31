#version 330 core 

in vec2 UV;
in vec4 w_color;

out vec4 outColor;

uniform sampler2D myTex;

void main()
{
    outColor = w_color;
    //outColor = texture(myTex, UV);
}