// Author:	Fredrik Präntare <prantare@gmail.com>
// Date:	11/26/2016
#version 450 core

layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPosition;

void main(){
	worldPosition = vec3(model * vec4(position, 1));
	gl_Position = projection * view * vec4(worldPosition, 1);
}