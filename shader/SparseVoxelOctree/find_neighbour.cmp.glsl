#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numNode;
uniform int u_start;
uniform int u_voxelDim;
uniform int u_level;
uniform int u_levelVoxelSize;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(r32ui) uniform uimageBuffer u_octreeNodeNeighXIdx;
layout(r32ui) uniform uimageBuffer u_octreeNodeNeighYIdx;
layout(r32ui) uniform uimageBuffer u_octreeNodeNeighZIdx;

#include "utils.glsl"

bool find(uvec3 loc, out uint neigh_idx)
{
	uvec3 umin = uvec3(0);
	int childIdx = 0;
	ivec3 offset;
	uint voxelDim = u_voxelDim;
	bool bFlag = true;
	uint node = imageLoad(u_octreeNodeIdx, childIdx).r;

	for (int i = 0; i < u_level; ++i)
	{
		voxelDim /= 2;
		if ((node & NODE_MASK_CHILD) == 0)
		{
			bFlag = false;
			break;
		}
		childIdx = int(node & NODE_MASK_INDEX);  //mask out flag bit to get child idx

		offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		childIdx += offset.x + 2 * offset.y + 4 * offset.z;
		umin += voxelDim * offset;

		node = imageLoad(u_octreeNodeIdx, childIdx).r;
	}
	
	if (bFlag)
	{
		bFlag = (node & NODE_MASK_CHILD) == 1;
		neigh_idx = childIdx;
	}

	return bFlag;
}

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numNode)
		return;

	uint nodeIdx = imageLoad(u_octreeNodeIdx, u_start + int(thxId)).r;
	if ((nodeIdx & NODE_MASK_CHILD) == 0)
	{
		return;
	}

	imageStore(u_octreeNodeIdx, u_start + int(thxId), uvec4(NODE_MASK_CHILD, 0, 0, 0));
	uint centerU = nodeIdx & NODE_MASK_INDEX;
	uvec3 center = uintXYZ10ToVec3(centerU);

	uint neigh_x = 0;
	uint neigh_y = 0;
	uint neigh_z = 0;
	uint neigh_idx;

	if (center.x + u_levelVoxelSize < u_voxelDim)
	{
		uvec3 right = center + uvec3(u_levelVoxelDim, 0, 0);
		if (find(right, neigh_idx))
		{
			neigh_x = neigh_idx;
		}
	}

	if (center.y + u_levelVoxelSize < u_voxelDim)
	{
		uvec3 up = center + uvec3(0, u_levelVoxelDim, 0);
		if (find(up, neigh_idx))
		{
			neigh_y = neigh_idx;
		}
	}

	if (center.z + u_levelVoxelSize < u_voxelDim)
	{
		uvec3 front = center + uvec3(0, 0, u_levelVoxelDim);
		if (find(front, neigh_idx))
		{
			neigh_z = neigh_idx;
		}
	}

	imageStore(u_octreeNodeNeighXIdx, u_start + int(thxId), uvec4(neigh_x, 0, 0, 0));
	imageStore(u_octreeNodeNeighYIdx, u_start + int(thxId), uvec4(neigh_y, 0, 0, 0));
	imageStore(u_octreeNodeNeighZIdx, u_start + int(thxId), uvec4(neigh_z, 0, 0, 0));
}