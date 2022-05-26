#version 330 core
out vec4 outColor;

in vec2 texCoords;

uniform sampler2D noteTexture;
uniform float amount;

void main()
{
    vec4 col = texture(noteTexture, texCoords);
    outColor = vec4(col.rgb, amount * col.a);
}
