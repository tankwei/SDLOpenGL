#version 320 es

in mediump vec3 color;
out mediump vec4 frag_color;

void main(){
	frag_color = vec4(color, 1.0); 
}