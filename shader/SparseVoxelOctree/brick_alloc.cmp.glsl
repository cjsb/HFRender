#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_num;
uniform int u_brickPoolDim;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;

//atomic counter 
layout(binding = 0, offset = 0) uniform atomic_uint u_allocCount;

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_num)
		return;

	int brickDim = u_brickPoolDim / 3;
	uint childIdx = imageLoad(u_octreeNodeIdx, thxId).r;
	if ((childIdx & NODE_MASK_CHILD) != 0)
	{
		uint offset = atomicCounterIncrement(u_allocCount);
		uvec4 brickIdx = uvec4(0);
		brickIdx.x = offset % brickDim;
		brickIdx.y = (offset / brickDim) % brickDim;
		brickIdx.z = offset / (brickDim * brickDim);
		brickIdx *= 3;
		imageStore(u_octreeNodeBrickIdx, thxId, brickIdx);
	}
}