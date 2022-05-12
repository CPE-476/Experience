#version 330 core

out vec4 outColor;

in float Height;
in vec3 Position;

void main()
{
    vec3 bottom = vec3(0.676, 0.691, 0.484);
    vec3 top = vec3(0.459, 0.525, 0.275);
    vec3 dirt = vec3(0.25, 0.129, 0.000);
    //bottom = vec3(1, 0, 0);
    //top = vec3(0, 1, 0);  //uncomment for RGB debugger
    //dirt = vec3(0, 0, 1); 
    float h = (Height+7.8) / 10;  // shift and scale the height in to a grayscale value
    vec3 amount;

    if (h < 0.5)
        {
            amount = (bottom * h * 2.0) +  dirt * (0.5 - h) * 2.0;
        }
    else
        {
            amount = top * (h - 0.5) * 2.0 + bottom * (1.0 - h) * 2.0;          
        }

    //vec3 amount = mix(bottom, top, h);
    
    outColor = vec4(amount, 1.0f);
}
