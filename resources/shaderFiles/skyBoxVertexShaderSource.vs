#version 320 es
layout (location = 0) in mediump vec3 aPos;
out mediump vec3 TexCoord;
uniform mat4 view;
uniform mat4 projection;
void main(){
gl_Position = projection * view * vec4(aPos.x, aPos.y, aPos.z, 1.0);
TexCoord=aPos;
}