#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 texCoords;
uniform samplerCube cubemap;

uniform float threshold;

void main()
{
    FragColor = texture(cubemap, texCoords);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > threshold)
    {
        BrightColor = vec4(FragColor.rgb, 1.0);
    }
    else
    {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
