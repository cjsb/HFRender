#version 450 core
out vec4 fragColor;

in vec2 frag_uv;
in vec3 frag_normal;

uniform sampler2D u_diffuseMap;
uniform vec3 u_diffuseColor;
uniform bool u_useMap;

struct DirectionLight {
	vec3 direction;
	vec3 color;
};

uniform DirectionLight light;

void main()
{ 
	if(u_useMap)
	{
		vec4 diffColor = texture(u_diffuseMap, frag_uv);
		fragColor = diffColor;
	}
	else
	{
		fragColor = vec4(u_diffuseColor,1);
	}
	
	fragColor = vec4(max(dot(-light.direction, frag_normal),0)*light.color*fragColor.rgb,1);
}