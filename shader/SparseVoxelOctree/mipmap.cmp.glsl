#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numNode;
uniform int u_start;
uniform int u_level;
uniform int u_octreeLevel;

uniform vec4 u_emptyColor;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;
layout(r32ui) uniform uimage3D u_octreeBrickValue;

#define NODE_MASK_CHILD 0x80000000
#define NODE_MASK_INDEX 0x7FFFFFFF

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

const float gaussianWeight[4] = { 0.25, 0.125, 0.0625, 0.03125 };

uint childIndices[8] = {0,0,0,0,0,0,0,0};
uvec4 childBrickIndices[8] = { uvec4(0),uvec4(0),uvec4(0),uvec4(0),uvec4(0),uvec4(0),uvec4(0),uvec4(0) };

void loadChildTile(int chlidIdx) {
	for (int i = 0; i < 8; ++i) {
		childIndices[i] = imageLoad(u_octreeNodeIdx, chlidIdx + i).r;
		childBrickIndices[i] = imageLoad(u_octreeNodeBrickIdx, chlidIdx + i);
	}
}

vec4 getColor(ivec3 pos) {
	ivec3 childPos = ivec3(round(vec3(pos) / 4.0));
	int offset = childPos.x + 2 * childPos.y + 4 * childPos.z;
	/*if ((childIndices[offset] & NODE_MASK_INDEX) == 0 && u_level != u_octreeLevel - 2)
	    return u_emptyColor;*/
	//叶子节点可能为空
	if ((childIndices[offset] & NODE_MASK_CHILD) == 0)
	{
		return u_emptyColor;
	}
		
	ivec3 localPos = pos - 2 * childPos;
	ivec3 childBrickAddress = ivec3(childBrickIndices[offset].xyz);
	uint val = imageLoad(u_octreeBrickValue, childBrickAddress + localPos).r;
	return convRGBA8ToVec4(val);
}

vec4 mipmapIsotropic(ivec3 pos) {
	vec4 col = vec4(0);
	float weightSum = 0.0;

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			for (int z = -1; z <= 1; ++z) {

				ivec3 lookupPos = pos + ivec3(x, y, z);

				if (lookupPos.x >= 0 && lookupPos.y >= 0 && lookupPos.z >= 0 &&
					lookupPos.x <= 4 && lookupPos.y <= 4 && lookupPos.z <= 4)
				{
					int manhattanDist = abs(x) + abs(y) + abs(z);
					float weight = gaussianWeight[manhattanDist];
					vec4 lookupColor = getColor(lookupPos);
					if (lookupColor.x != u_emptyColor.x || 
						lookupColor.y != u_emptyColor.y || 
						lookupColor.z != u_emptyColor.z)
					{
						col += weight * lookupColor;
						weightSum += weight;
					}
				}
			}
		}
	}

	if (weightSum > 0)
	{
		col /= weightSum;
	}

	return col;
}

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numNode)
		return;

	uint node = imageLoad(u_octreeNodeIdx, u_start + int(thxId)).r;
	uint chlidIdx = node & NODE_MASK_INDEX;
	if (chlidIdx == 0)
	{
		return;
	}

	loadChildTile(int(chlidIdx));

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, u_start + int(thxId));
	ivec3 brickAddress = ivec3(brick_idx.xyz);

	for (int x = 0; x <= 4; x++)
	{
		for (int y = 0; y <= 4; y++)
		{
			for (int z = 0; z <= 4; z++)
			{
				if (x % 2 == 0 && y % 2 == 0 && z % 2 == 0)
				{
					ivec3 coord = ivec3(x, y, z);
					vec4 color = mipmapIsotropic(coord);
					uint color_v = convVec4ToRGBA8(color);

					ivec3 loc = coord / 2;
					imageStore(u_octreeBrickValue, brickAddress + loc, uvec4(color_v, 0, 0, 0));
				}
			}
		}
	}
}