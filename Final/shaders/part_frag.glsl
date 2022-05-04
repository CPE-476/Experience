#version 330 core 

in vec2 UV;
in vec4 partColor;

out vec4 outColor;

uniform sampler2D myTex;

void main()
{
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
    //outColor = partColor;
    vec4 color = texture(myTex, UV) * partColor;
    if(color.a < 0.1)
        discard;
    outColor = color;
}