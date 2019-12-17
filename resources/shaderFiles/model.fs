#version 320 es
out mediump vec4 FragColor;

in mediump vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);//vec4(0.3,0.6,0.3,1.0);
    //
}