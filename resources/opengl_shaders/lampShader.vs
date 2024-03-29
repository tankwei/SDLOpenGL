#version 320 es

in mediump vec3 tposition; // name matches string in shader pgm
in mediump vec2 texCoord;  // name matches string in shader pgm, not used here

uniform mat4 transform; // model matrix
uniform mat4 view;
uniform mat4 projection;

void main(){
	gl_Position = projection * view * transform * vec4(tposition, 1.0);
}