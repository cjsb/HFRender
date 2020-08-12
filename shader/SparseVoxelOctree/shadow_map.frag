#version 450 core
out vec4 fragColor;

in vec3 v_vertex;
in vec3 v_normal;

void main()
{
	fragColor = vec4(v_vertex,1);
}