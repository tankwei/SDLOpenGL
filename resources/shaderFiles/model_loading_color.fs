#version 320 es
out mediump vec4 FragColor;
in mediump vec3 acolor;

void main()
{
    FragColor = vec4(acolor,1.0);
}