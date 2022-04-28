#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform float ftime;

void main(){
    
    float dist = distance(gl_FragCoord.xy, vec2(1080+400*sin(ftime),720+400*cos(ftime)));
    
    vec3 yellow = vec3(1, 1, 0);
    vec3 red = vec3(0.5, 0, 0);
    vec3 texColor = texture(texBuf, texCoord).rgb;
    texColor += (1-dist/300) * yellow;
    
    //TODO modify to show this is a 2D image

    color = vec4(texColor, 1.0);
}

/*void main(){
    vec3 blue = vec3(0, 0, 0.5);
    vec3 red = vec3(0.5, 0, 0);
    vec3 texColor = texture(texBuf, vec2(texCoord.x + ftime, texCoord.y + sin(gl_FragCoord.x/500))).rgb;
    //TODO modify to show this is a 2D image
    if (gl_FragCoord.x > 1080)
        texColor -= blue + red;
    color = vec4(texColor, 1.0);
}*/

/*void main(){

    vec3 texColor = texture( texBuf, texCoord ).rgb;
    //TODO modify to show this is a 2D image
    color = vec4(texColor, 1.0);

}*/
