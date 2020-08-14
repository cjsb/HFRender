#version 450 core
layout(location = 0) in vec3 position;

out vec2 frag_uv;

void main()
{
	frag_uv = position.xy;
	vec3 pos = position*2-1;
    gl_Position = vec4(pos.xy, 0, 1);
}