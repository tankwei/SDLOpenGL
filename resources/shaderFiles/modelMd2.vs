#version 320 es
layout (location = 0) in mediump vec4 aPos;
layout(location = 1) in mediump vec3 glNormal;
layout(location = 2) in mediump vec2 aTexCoord;
out mediump vec2 TexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
gl_Position = projection * view * model * aPos;
TexCoord = aTexCoord;
}