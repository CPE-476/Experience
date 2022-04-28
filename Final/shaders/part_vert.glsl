#version  330 core
layout(location = 0) in vec3 VertexPosition;
layout(location = 2) in vec3 InstancedPosition;
layout(location = 3) in vec4 color;
layout(location = 4) in float scale;

out vec2 UV;
out vec4 partColor;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

uniform vec3 CameraRight;
uniform vec3 CameraUp;

void main()
{
    vec3 VertexBillboard = vec3(0.0f, 0.5f, 0.0f) + CameraRight * VertexPosition.x * 1.0f + CameraUp * VertexPosition.y * 1.0f;
    
    mat4 scaleMat = mat4(
        vec4(scale,0.0,0.0,0.0),
        vec4(0.0,scale,0.0,0.0),
        vec4(0.0,0.0,scale,0.0),
        vec4(0.0,0.0,0.0,0.0));

    gl_Position = Projection * View * ((Model * scaleMat) * vec4(VertexBillboard, 1.0) + vec4(InstancedPosition, 1.0));
    //gl_Position = Projection * View * Model * vec4(VertexPosition, 1.0);
    //gl_Position = vec4(VertexPosition, 1.0);
    UV = VertexPosition.xy + vec2(0.5, 0.5);
    partColor = color;

}