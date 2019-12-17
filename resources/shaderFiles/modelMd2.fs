#version 320 es
out mediump vec4 FragColor;
in mediump vec2 TexCoord;
uniform sampler2D texture1;
void main(){
FragColor = vec4(texture(texture1, TexCoord.xy).rgb,1.0f);//vec4(1.0f, 0.5f, 0.2f, 1.0f);

}