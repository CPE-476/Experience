#version 330 core
uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

out vec4 Outcolor;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos) {
    float percentShadow = 0.0;
	//1: shift the coordinates from -1, 1 to 0 ,1
    vec3 projCoord = (LSfPos.xyz + vec3(1.0)) * 0.5;
    float bias = 0.005;
    float curDepth = projCoord.z;
	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy
    float lightDepth = texture(shadowDepth, projCoord.xy).r;
	//3: compare to the current depth (.z) of the projected depth
	//4: return 1 if the point is shadowed
    //first compute texture scale
    vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);
    //index into the texture using this scale to offset by 1-2 fragments:
    for (int i=-2; i <= 2; i++) {
        for (int j=-2; j <= 2; j++) {
          lightDepth = texture(shadowDepth, projCoord.xy+vec2(i, j)*texelScale).r;
          if (curDepth > lightDepth)
            percentShadow += 1.0;
        }
      }
    return percentShadow/25.0;
}

void main() {

  float Shade;
  float amb = 0.3;

  vec4 BaseColor = vec4(in_struct.vColor, 1);
  vec4 texColor0 = texture(Texture0, in_struct.vTexCoord);

  Shade = TestShadow(in_struct.fPosLS);

  Outcolor = amb*(texColor0) + (1.0-Shade)*texColor0*BaseColor;
}

