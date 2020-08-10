#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_allocStart;
uniform int u_num;

layout(r32ui) coherent uniform uimageBuffer u_octreeNodeIdx;
layout(r32ui) coherent uniform uimage3D u_octreeBrickColor;
layout(r32ui) coherent uniform uimage3D u_octreeBrickNormal

void main()
{
	uint offset;
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_num)
		return;

	imageStore(u_octreeNodeIdx, int(u_allocStart + thxId), uvec4(0, 0, 0, 0));
	imageStore(u_octreeBrickColor, int(u_allocStart + thxId), uvec4(0, 0, 0, 0));
	imageStore(u_octreeBrickNormal, int(u_allocStart + thxId), uvec4(0, 0, 0, 0));
}