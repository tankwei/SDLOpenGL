#version 320 es
out mediump vec4 FragColor;
in mediump vec3 TexCoord;
uniform samplerCube skybox;
void main(){
    FragColor = texture(skybox,TexCoord);
}