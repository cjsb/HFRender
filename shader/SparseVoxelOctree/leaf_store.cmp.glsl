#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;

layout(r32ui) uniform volatile uimage3D u_octreeBrickColor;
layout(r32ui) uniform volatile uimage3D u_octreeBrickNormal;

layout(rgb10_a2ui) uniform uimageBuffer u_voxelListPos;
layout(rgba8) uniform imageBuffer u_voxelListColor;
layout(rgba8) uniform imageBuffer u_voxelListNormal;

uniform int u_voxelDim;
uniform int u_octreeLevel;
uniform int u_numVoxelFrag;

#include "utils.glsl"

//UINT atomic running average method
//From OpenGL Insight ch. 22
void imageAtomicRGBA8Avg(vec4 val, ivec3 coord, layout(r32ui) volatile uimage3D image)
{
	val.rgb *= 255.0;
	val.a = 1;

	uint newVal = convVec4ToRGBA8(val);
	uint prev = 0;
	uint cur;

	while ((cur = imageAtomicCompSwap(image, coord, prev, newVal)) != prev)
	{
		prev = cur;
		vec4 rval = convRGBA8ToVec4(cur);
		rval.xyz = rval.xyz * rval.w;
		vec4 curVal = rval + val;
		curVal.xyz /= curVal.w;
		newVal = convVec4ToRGBA8(curVal);
	}
}

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numVoxelFrag)
		return;

	uvec4 loc = imageLoad(u_voxelListPos, int(thxId));
	int childIdx = 0;
	uint node;
	ivec3 offset;

	uint voxelDim = u_voxelDim;
	uvec3 umin = uvec3(0, 0, 0);

	node = imageLoad(u_octreeNodeIdx, childIdx).r;
	for (int i = 0; i < u_octreeLevel; ++i)
	{
		voxelDim /= 2;
		childIdx = int(node & NODE_MASK_INDEX);  //mask out flag bit to get child idx
		if (childIdx == 0)
		{
			break;
		}

		offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		childIdx += offset.x + 2 * offset.y + 4 * offset.z;
		umin += voxelDim * offset;

		node = imageLoad(u_octreeNodeIdx, childIdx).r;
	}

	//获取最后一层voxel（octreeLevel）在叶子节点中的offset
	offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
	offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
	offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, childIdx);
	//store VoxelColors in brick corners
	ivec3 brick_coord = ivec3(brick_idx.xyz) + 2 * offset;

	vec4 color = imageLoad(u_voxelListColor, int(thxId));
	vec4 normal = imageLoad(u_voxelListNormal, int(thxId));

	//Use a atomic running average method to prevent buffer saturation
	//From OpenGL Insight ch. 22
	imageAtomicRGBA8Avg(color, brick_coord, u_octreeBrickColor);
	imageAtomicRGBA8Avg(normal, brick_coord, u_octreeBrickNormal);
}