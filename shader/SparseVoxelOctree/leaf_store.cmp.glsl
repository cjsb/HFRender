#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(r32ui) uniform uimageBuffer u_octreeIdx;
layout(r32ui) uniform uimageBuffer u_octreeKd;

layout(rgb10_a2ui) uniform uimageBuffer u_voxelPos;
layout(rgba8) uniform imageBuffer u_voxelKd;

uniform int u_voxelDim;
uniform int u_octreeLevel;
uniform int u_numVoxelFrag;

void imageAtomicRGBA8Avg(vec4 val, int coord, layout(r32ui) uimageBuffer buf);
uint convVec4ToRGBA8(vec4 val);
vec4 convRGBA8ToVec4(uint val);

void main()
{
	uint thxId = gl_GlobalInvocationID.y * 1024 + gl_GlobalInvocationID.x;

	if (thxId >= u_numVoxelFrag)
		return;
	uvec4 loc = imageLoad(u_voxelPos, int(thxId));
	int childIdx = 0;
	uint node, subnode;
	bool bFlag = true;

	uint voxelDim = u_voxelDim;
	uvec3 umin = uvec3(0, 0, 0);

	node = imageLoad(u_octreeIdx, childIdx).r;

	for (int i = 0; i < u_octreeLevel; ++i)
	{
		voxelDim /= 2;
		if ((node & 0x80000000) == 0)
		{
			bFlag = false;
			break;
		}
		childIdx = int(node & 0x7FFFFFFF);  //mask out flag bit to get child idx

		subnode = clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		subnode += 4 * clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		subnode += 2 * clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);
		childIdx += int(subnode);

		umin.x += voxelDim * clamp(int(1 + loc.x - umin.x - voxelDim), 0, 1);
		umin.y += voxelDim * clamp(int(1 + loc.y - umin.y - voxelDim), 0, 1);
		umin.z += voxelDim * clamp(int(1 + loc.z - umin.z - voxelDim), 0, 1);

		node = imageLoad(u_octreeIdx, childIdx).r;
	}

	vec4 color = imageLoad(u_voxelKd, int(thxId));

	//Use a atomic running average method to prevent buffer saturation
	//From OpenGL Insight ch. 22
	imageAtomicRGBA8Avg(color, childIdx, u_octreeKd);
}

//UINT atomic running average method
//From OpenGL Insight ch. 22
vec4 convRGBA8ToVec4(in uint val)
{
	return vec4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U),
		float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
}

uint convVec4ToRGBA8(in vec4 val)
{
	return (uint(val.w) & 0x000000FF) << 24U | (uint(val.z) & 0x000000FF) << 16U | (uint(val.y) & 0x000000FF) << 8U | (uint(val.x) & 0x000000FF);
}

void imageAtomicRGBA8Avg(vec4 val, int coord, layout(r32ui) uimageBuffer buf)
{
	val.rgb *= 255.0;
	val.a = 1;

	uint newVal = convVec4ToRGBA8(val);
	uint prev = 0;
	uint cur;

	while ((cur = imageAtomicCompSwap(buf, coord, prev, newVal)) != prev)
	{
		prev = cur;
		vec4 rval = convRGBA8ToVec4(cur);
		rval.xyz = rval.xyz * rval.w;
		vec4 curVal = rval + val;
		curVal.xyz /= curVal.w;
		newVal = convVec4ToRGBA8(curVal);
	}
}