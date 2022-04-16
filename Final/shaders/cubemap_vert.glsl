#version 330 core

layout (location = 0) in vec3 vertexPosition;

out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // Cubemaps use vertex position for texturing.
    texCoords = vertexPosition;
    // Normalized coordinates for depth rendering behind everything.
    gl_Position = (projection * view * vec4(vertexPosition, 1.0)).xyww;
}
// For more info, read LearnOpenGL cubemap chapter.
