#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in mat4 instanceMatrix;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform bool isT;

void main()
{
    if(!isT)
        gl_Position = lightSpaceMatrix * instanceMatrix * vec4(aPos, 1.0);
    else
        gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
} 
