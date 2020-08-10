#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numVoxelFrag;
uniform int u_level;
uniform int u_voxelDim;

layout(rgb10_a2ui) uniform uimageBuffer u_voxelListPos;
layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;

#include "utils.glsl"

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numVoxelFrag)
		return;

	uvec3 umin;
	uvec4 loc;
	int childIdx = 0;
	uint node, subnode;
	uint voxelDim = u_voxelDim;
	bool bFlag = true;

	//Get the voxel coordinate of voxel loaded by this thread
	loc = imageLoad(u_voxelListPos, int(thxId));

	//decide max and min coord for the root node
	umin = uvec3(0, 0, 0);

	node = imageLoad(u_octreeNodeIdx, childIdx).r;

	for (int i = 0; i < u_level; ++i)
	{
		voxelDim /= 2;
		if ((node & NODE_MASK_CHILD) == 0)
		{
			bFlag = false;
			break;
		}
		childIdx = int(node & NODE_MASK_INDEX);  //mask out flag bit to get child idx

		subnode = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		subnode += 4 * clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		subnode += 2 * clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);
		childIdx += int(subnode);

		umin.x += voxelDim * clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		umin.y += voxelDim * clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		umin.z += voxelDim * clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		node = imageLoad(u_octreeNodeIdx, childIdx).r;
	}
	if (bFlag)
	{
		node |= NODE_MASK_CHILD; //set the most significant bit
		imageStore(u_octreeNodeIdx, childIdx, uvec4(node, 0, 0, 0));
	}
}