#version 450 core
out vec4 fragColor;

in vec2 frag_uv;

uniform sampler2D u_shadowMap;

void main()
{
	vec3 pos = texture(u_shadowMap, frag_uv).xyz;
	pos = pos*0.5+0.5;
	fragColor = vec4(pos,1);
}