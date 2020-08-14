#version 450 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

flat out vec3 center;

void main(){
	center = normal;
	gl_Position = projection * view * model * vec4(position, 1); 
}