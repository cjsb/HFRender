// Author:	Fredrik Präntare <prantare@gmail.com>
// Date:	11/26/2016
#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPositionFrag;
out vec3 normalFrag;

void main(){
	worldPositionFrag = vec3(model * vec4(position, 1));
	normalFrag = normalize(mat3(transpose(inverse(model))) * normal);
	gl_Position = projection * view * vec4(worldPositionFrag, 1);
}