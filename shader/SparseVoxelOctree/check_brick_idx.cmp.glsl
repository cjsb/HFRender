#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numOctreeNode;
uniform int u_brickPoolDim;

layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;

//atomic counter 
layout(binding = 0, offset = 0) uniform atomic_uint u_allocCount;


void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numOctreeNode)
		return;

	uvec4 brickIdx = imageLoad(u_octreeNodeBrickIdx, int(thxId));
	if (brickIdx.x >= u_brickPoolDim|| brickIdx.y >= u_brickPoolDim|| brickIdx.z >= u_brickPoolDim)
	{
		atomicCounterIncrement(u_allocCount);
	}
}