#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPositionGeom;
out vec3 normalGeom;

void main(){
	worldPositionGeom = vec3(model * vec4(position, 1));
	normalGeom = normalize(mat3(transpose(inverse(model))) * normal);
	gl_Position = projection * view * vec4(worldPositionGeom, 1);
}