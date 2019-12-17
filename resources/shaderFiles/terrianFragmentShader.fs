#version 320 es
out mediump vec4 FragColor;
in mediump vec2 f_TexCoord;
uniform sampler2D ourTexture;
void main(){
FragColor = texture(ourTexture,f_TexCoord);
}