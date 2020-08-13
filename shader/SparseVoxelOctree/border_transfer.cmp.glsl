#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform int u_numNode;
uniform int u_start;
uniform int u_axis;

layout(r32ui) uniform uimageBuffer u_octreeNodeNeighbourIdx;
layout(rgb10_a2ui) uniform uimageBuffer u_octreeNodeBrickIdx;
layout(r32ui) uniform uimage3D u_octreeBrickValue;

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2


vec4 convRGBA8ToVec4(in uint val)
{
	return vec4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U),
		float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
}

uint convVec4ToRGBA8(in vec4 val)
{
	return (uint(val.w) & 0x000000FF) << 24U | (uint(val.z) & 0x000000FF) << 16U | (uint(val.y) & 0x000000FF) << 8U | (uint(val.x) & 0x000000FF);
}

uint getFinalVal(uint borderVal, uint neighbourBorderVal) {
	vec4 borderVec = convRGBA8ToVec4(borderVal);
	vec4 neighbourBorderVec = convRGBA8ToVec4(neighbourBorderVal);
	vec4 col = 0.5 * (borderVec + neighbourBorderVec);
	return convVec4ToRGBA8(col);
}

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;
	if (thxId >= u_numNode)
		return;

	uint neighbourIdx = imageLoad(u_octreeNodeNeighbourIdx, u_start + int(thxId)).r;
	if (neighbourIdx == 0)
	{
		return;
	}

	uvec4 brick_idx = imageLoad(u_octreeNodeBrickIdx, u_start + int(thxId));
	ivec3 brickAddress = ivec3(brick_idx.xyz);

	uvec4 neighbour_brick_idx = imageLoad(u_octreeNodeBrickIdx, int(neighbourIdx));
	ivec3 neighbour_brickAddress = ivec3(neighbour_brick_idx.xyz);

	if (u_axis == AXIS_X)
	{
		for (int y = 0; y <= 2; ++y) 
		{
			for (int z = 0; z <= 2; ++z)
			{
				ivec3 offset = ivec3(2, y, z);
				ivec3 nOffset = ivec3(0, y, z);
				uint borderVal = imageLoad(u_octreeBrickValue, brickAddress + offset).r;
				uint neighbourBorderVal = imageLoad(u_octreeBrickValue, neighbour_brickAddress + nOffset).r;

				uint finalVal = getFinalVal(borderVal, neighbourBorderVal);
				imageStore(u_octreeBrickValue, brickAddress + offset, uvec4(finalVal, 0, 0, 0));
				imageStore(u_octreeBrickValue, neighbour_brickAddress + nOffset, uvec4(finalVal, 0, 0, 0));
			}
		}
	}
	else if (u_axis == AXIS_Y)
	{
		for (int x = 0; x <= 2; ++x)
		{
			for (int z = 0; z <= 2; ++z)
			{
				ivec3 offset = ivec3(x, 2, z);
				ivec3 nOffset = ivec3(x, 0, z);
				uint borderVal = imageLoad(u_octreeBrickValue, brickAddress + offset).r;
				uint neighbourBorderVal = imageLoad(u_octreeBrickValue, neighbour_brickAddress + nOffset).r;

				uint finalVal = getFinalVal(borderVal, neighbourBorderVal);
				imageStore(u_octreeBrickValue, brickAddress + offset, uvec4(finalVal, 0, 0, 0));
				imageStore(u_octreeBrickValue, neighbour_brickAddress + nOffset, uvec4(finalVal, 0, 0, 0));
			}
		}
	}
	else if (u_axis == AXIS_Z)
	{
		for (int x = 0; x <= 2; ++x)
		{
			for (int y = 0; y <= 2; ++y)
			{
				ivec3 offset = ivec3(x, y, 2);
				ivec3 nOffset = ivec3(x, y, 0);
				uint borderVal = imageLoad(u_octreeBrickValue, brickAddress + offset).r;
				uint neighbourBorderVal = imageLoad(u_octreeBrickValue, neighbour_brickAddress + nOffset).r;

				uint finalVal = getFinalVal(borderVal, neighbourBorderVal);
				imageStore(u_octreeBrickValue, brickAddress + offset, uvec4(finalVal, 0, 0, 0));
				imageStore(u_octreeBrickValue, neighbour_brickAddress + nOffset, uvec4(finalVal, 0, 0, 0));
			}
		}
	}
}