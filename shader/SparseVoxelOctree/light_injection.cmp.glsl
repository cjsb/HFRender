#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(r32ui) uniform uimageBuffer u_octreeNodeIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;

layout(r32ui) uniform uimage3D u_octreeBrickColor;
layout(r32ui) uniform uimage3D u_octreeBrickIrradiance;

uniform sampler2D u_shadowMap;

uniform int u_voxelDim;
uniform int u_octreeLevel;

struct DirectionLight {
	vec3 direction;
	vec3 color;
};

uniform DirectionLight u_light;

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

void main()
{
	ivec2 mapSize = textureSize(u_lightMap, 0);
	if (gl_GlobalInvocationID.x >= mapSize.x || gl_GlobalInvocationID.y >= mapSize.y)
		return;
	
	vec2 uv = vec2(gl_GlobalInvocationID.x / float(mapSize.x), gl_GlobalInvocationID.y / float(mapSize.y));
	vec3 pos = texture(u_shadowMap, uv).xyz; //-1~1
	if (pos.x == 0 && pos.y == 0 && pos.z == 0)
	{
		return;
	}

	pos = pos * 0.5 + 0.5; //0~1
	if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
		pos.x > 1 || pos.y > 1 || pos.z > 1)
	{
		return;
	}

	uvec3 loc = uvec3(pos * u_voxelDim);
	int childIdx = 0;
	uint node;
	ivec3 offset;
	uint voxelDim = u_voxelDim;
	uvec3 umin = uvec3(0, 0, 0);

	node = imageLoad(u_octreeNodeIdx, childIdx).r;
	//这里需要设置成最后一层 u_octreeLevel-1
	for (int i = 0; i < u_octreeLevel - 1; ++i)
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
		return;
	}

	//得到最后一层的半径
	voxelDim /= 2;

	//获取最后一层voxel（octreeLevel）在叶子节点中的offset
	offset.x = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
	offset.y = clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
	offset.z = clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, childIdx);
	ivec3 brick_coord = ivec3(brick_idx.xyz) + 2 * offset;

	uint voxelColor_v = imageLoad(u_octreeBrickColor, brick_coord).r;
	vec4 voxelColor = convRGBA8ToVec4(voxelColor_v);
	voxelColor.rgb /= 255.0;

	vec4 reflectedRadiance = vec4(u_light.color * voxelColor.rgb, 1);
	reflectedRadiance.rgb *= 255.0;
	uint reflectedRadiance_v = convVec4ToRGBA8(reflectedRadiance);
	
	imageStore(u_octreeBrickIrradiance, brick_coord, uvec4(reflectedRadiance_v, 0, 0, 0));
}