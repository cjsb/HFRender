#version 450 core
out vec4 fragColor;

flat in vec3 worldPosition;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;
layout(r32ui) uniform uimage3D u_octreeBrickColor;

uniform int u_voxelDim;
uniform int u_level;

#define NODE_MASK_CHILD 0x80000000
#define NODE_MASK_INDEX 0x7FFFFFFF

vec4 convRGBA8ToVec4(in uint val)
{
	return vec4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U),
		float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
}

void main()
{
	uvec3 umin;
	uvec4 loc;
	int childIdx = 0;
	uint node;
	ivec3 offset;
	uint voxelDim = u_voxelDim;
	bool bFlag = true;

	loc = uvec4(worldPosition*u_voxelDim,0);

	//decide max and min coord for the root node
	umin = uvec3(0);

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

		offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		childIdx += offset.x + 2 * offset.y + 4 * offset.z;
		umin += voxelDim * offset;

		node = imageLoad(u_octreeNodeIdx, childIdx).r;
	}

	if (!bFlag)
	{
		discard;
	}
	else
	{
		voxelDim /= 2;

		//获取voxel在节点中的offset
		offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, childIdx);
		ivec3 brick_coord = ivec3(brick_idx.xyz) + 2 * offset;

		uint voxelColor_v = imageLoad(u_octreeBrickColor, brick_coord).r;
		vec4 voxelColor = convRGBA8ToVec4(voxelColor_v);
		voxelColor.rgb /= 255.0;
		fragColor = vec4(voxelColor.rgb, 1);
		if(voxelColor.r==0&&voxelColor.g==0&&voxelColor.b==0)
		{
			fragColor = vec4(1,0,0,1);
			//discard;
		}
	}
}