#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform vec3 objColor;

uniform sampler2D tex;
uniform sampler2D tex2;

uniform float dn;
void main()
{
vec4 tcol = texture(tex, vertex_tex);
vec4 tcoln = texture(tex2, vertex_tex);
if(objColor == vec3(0, 0, 0))
    color = tcoln;
else
    color = vec4(objColor, 1);//*0.5 + 0.5 * vec4(vertex_pos, 1);
}
