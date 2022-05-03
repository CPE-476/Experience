#version 330 core

in vec2 texCoord;
out vec4 color;
//uniform sampler2D texBuf;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;
//ignored for now
uniform vec3 Ldir;

/* just pass through the texture color we will add to this next lab */
void main(){
    
    vec3 lightD = vec3(1,1,0);
    
    vec3 tPos = texture( gColorSpec, texCoord ).rgb;
    vec3 tNor = texture( gNormal, texCoord ).rgb;
    vec3 tColor = texture( gColorSpec, texCoord ).rgb;
    float dc = max(dot(normalize(tNor), normalize(lightD)), 0);
    vec3 amb = 0.1 * tColor;
   color = vec4(tColor * dc + amb, 1.0);
   //if (abs(tColor.r) > 0.01 || abs(tColor.g) > 0.01)
   //	color = vec4(0.9, 0.9, 0.9, 1.0);

}
