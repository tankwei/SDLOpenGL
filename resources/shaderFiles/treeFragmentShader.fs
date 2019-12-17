#version 320 es
out mediump vec4 FragColor;
in mediump vec2 f_TexCoord;
uniform sampler2D ourTexture;
void main(){
mediump vec4 texColor = texture(ourTexture,f_TexCoord);
mediump float lightValue = (texColor.r + texColor.g + texColor.b) / 3.0f;
if(lightValue < 0.1){
	discard;
}
else if(lightValue < 0.2){
   texColor.a = (lightValue - 0.05) / (0.2 - 0.05);
}
else{
   texColor.a = 1.0;
}
FragColor = texColor;
}