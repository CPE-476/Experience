#version 330 core

out vec4 outColor;

in float Height;
in vec3 Position;

void main()
{
    float h = (Height + 16)/32.0f;  // shift and scale the height in to a grayscale value
    vec3 amount = vec3(h, h, h);
    
    outColor = vec4(amount, 1.0f);
}
