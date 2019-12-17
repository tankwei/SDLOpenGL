#version 320 es

in mediump vec3 vertex_position; // name matches string in shader pgm
in mediump vec3 vertex_color;    // name matches string in shader pgm

out mediump vec3 color; // out: pass data to next render stage (fragment shader)

void main(){
	color = vertex_color;
	gl_Position = vec4(vertex_position, 1.0);
}