#version 320 es
layout (location = 0) in mediump vec3 aPos;
layout (location = 1) in mediump vec2 TexCoord;
out mediump vec2 f_TexCoord;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 offsets[100];
void main(){
gl_Position = projection * view * offsets[gl_InstanceID] * vec4(aPos.x,aPos.y,aPos.z, 1.0);
f_TexCoord=TexCoord;
}