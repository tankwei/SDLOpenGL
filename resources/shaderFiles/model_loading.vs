#version 320 es
layout (location = 0) in mediump vec3 aPos;
layout (location = 1) in mediump vec3 aNormal;
layout (location = 2) in mediump vec2 aTexCoords;

out mediump vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}