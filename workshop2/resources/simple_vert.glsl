#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec3 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragNor;
out vec3 WPos;

void main()
{
	gl_Position = P * V *M* vertPos;
	fragNor = (V* M * vec4(vertNor, 0.0)).xyz;
	WPos = vec3(V* M*vertPos);
}
