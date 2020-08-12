#version 450 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 v_vertex;
out vec3 v_normal;

void main()
{
    v_vertex = vec3(model * vec4(position, 1));
	v_normal = normalize(mat3(transpose(inverse(model))) * normal);
    gl_Position = projection * view * vec4(v_vertex, 1);
}