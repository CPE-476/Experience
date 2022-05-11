#version 330 core

out vec4 outColor;

in float Height;
in vec3 Position;

void main()
{
    vec3 bottom = vec3(0.676, 0.691, 0.484);
    vec3 top = vec3(0.559, 0.625, 0.375);
    vec3 blk = vec3(1.0, 1.0, 1.0);
    float h = (Height-0.5) / 5;  // shift and scale the height in to a grayscale value

    vec3 amount = mix(bottom, top, h);
    
    outColor = vec4(amount, 1.0f);
}
