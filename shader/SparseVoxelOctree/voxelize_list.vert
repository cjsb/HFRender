#version 450 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 model;

out vec3 v_vertex;
out vec3 v_normal;
out vec2 v_uv;

void main()
{
    v_vertex = vec3(model * vec4(position, 1));
	v_normal = normalize(mat3(transpose(inverse(model))) * normal);
    v_uv = uv;
    gl_Position = vec4( v_vertex, 1 );
}