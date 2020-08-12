#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numOctreeNode;
uniform int u_brickPoolDim;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;
layout(r32ui) uniform uimage3D u_octreeBrickColor;
layout(r32ui) uniform uimage3D u_octreeBrickNormal;
layout(r32ui) uniform uimage3D u_octreeBrickIrradiance;

//atomic counter 
layout(binding = 0, offset = 0) uniform atomic_uint u_allocCount;

#include "utils.glsl"

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numOctreeNode)
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

		vec4 clearNormal = vec4(0.5, 0.5, 0.5, 0.0);
		clearNormal.rgb *= 255.0;
		uint clearNormal_v = convVec4ToRGBA8(clearNormal);
		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				for (int z = 0; z < 3; z++)
				{
					ivec3 coord = ivec3(brickIdx.x + x, brickIdx.y + y, brickIdx.z + z);
					imageStore(u_octreeBrickColor, coord, uvec4(0));
					imageStore(u_octreeBrickNormal, coord, uvec4(clearNormal_v, 0, 0, 0));
					imageStore(u_octreeBrickIrradiance, coord, uvec4(0));
				}
			}
		}
	}
}