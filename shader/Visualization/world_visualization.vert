#version 450 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 frag_uv;
out vec3 frag_normal;

void main(){
	frag_uv = uv;
	frag_normal = normalize(mat3(transpose(inverse(model))) * normal);
	gl_Position = projection * view * model* vec4(position, 1);
}