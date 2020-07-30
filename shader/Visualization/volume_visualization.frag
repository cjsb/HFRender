#version 450 core

in vec3 worldPosition;

out vec4 color;

uniform sampler3D texture3D;

// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }

void main()
{
	vec3 coordinate = scaleAndBias(worldPosition);
	vec4 currentSample = textureLod(texture3D, coordinate, 0);
	if(currentSample.a>0)
	{
		color = currentSample;
		color.rgb = pow(color.rgb, vec3(1.0 / 2.2))+vec3(0.2);
	}
	else
	{
		discard;
	}
}