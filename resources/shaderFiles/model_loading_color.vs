#version 320 es
layout (location = 0) in mediump vec3 aPos;
layout (location = 1) in mediump vec3 aVectrexColor;

out mediump vec3 acolor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    acolor = aVectrexColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}