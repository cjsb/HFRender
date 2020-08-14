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

#define NODE_MASK_CHILD 0x80000000
#define NODE_MASK_INDEX 0x7FFFFFFF

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

const uvec3 childOffsets[8] = {
  uvec3(0, 0, 0),
  uvec3(1, 0, 0),
  uvec3(0, 1, 0),
  uvec3(1, 1, 0),
  uvec3(0, 0, 1),
  uvec3(1, 0, 1),
  uvec3(0, 1, 1),
  uvec3(1, 1, 1)
};

uint vec3ToUintXYZ10(uvec3 val) {
	return (uint(val.z) & 0x000003FF) << 20U
		| (uint(val.y) & 0x000003FF) << 10U
		| (uint(val.x) & 0x000003FF);
}

uvec3 uintXYZ10ToVec3(uint val) {
	return uvec3(uint((val & 0x000003FF)),
		uint((val & 0x000FFC00) >> 10U),
		uint((val & 0x3FF00000) >> 20U));
}

vec4 convRGBA8ToVec4(in uint val)
{
	return vec4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U),
		float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
}

uint convVec4ToRGBA8(in vec4 val)
{
	return (uint(val.w) & 0x000000FF) << 24U | (uint(val.z) & 0x000000FF) << 16U | (uint(val.y) & 0x000000FF) << 8U | (uint(val.x) & 0x000000FF);
}

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
		//叶子节点可能为空
		bFlag = (node & NODE_MASK_CHILD) != 0;
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
		uvec3 right = center + uvec3(u_levelVoxelSize, 0, 0);
		if (find(right, neigh_idx))
		{
			neigh_x = neigh_idx;
		}
	}

	if (center.y + u_levelVoxelSize < u_voxelDim)
	{
		uvec3 up = center + uvec3(0, u_levelVoxelSize, 0);
		if (find(up, neigh_idx))
		{
			neigh_y = neigh_idx;
		}
	}

	if (center.z + u_levelVoxelSize < u_voxelDim)
	{
		uvec3 front = center + uvec3(0, 0, u_levelVoxelSize);
		if (find(front, neigh_idx))
		{
			neigh_z = neigh_idx;
		}
	}

	imageStore(u_octreeNodeNeighXIdx, u_start + int(thxId), uvec4(neigh_x, 0, 0, 0));
	imageStore(u_octreeNodeNeighYIdx, u_start + int(thxId), uvec4(neigh_y, 0, 0, 0));
	imageStore(u_octreeNodeNeighZIdx, u_start + int(thxId), uvec4(neigh_z, 0, 0, 0));
}