#version 450 core
flat in int f_axis;   //indicate which axis the projection uses
flat in vec4 f_AABB;

in vec3 f_normal;
in vec3 f_pos;

//atomic counter 
layout (binding = 0, offset = 0) uniform atomic_uint u_voxelFragCount;

layout(rgb10_a2ui) uniform uimageBuffer u_voxelListPos;
layout(rgba8) uniform imageBuffer u_voxelListColor;
layout(rgba8) uniform imageBuffer u_voxelListNormal;

uniform vec3 u_Color;
uniform int u_voxelSize;
uniform int u_bStore; //0 for counting the number of voxel fragments
                      //1 for storing voxel fragments
void main()
{
    if( f_pos.x < f_AABB.x || f_pos.y < f_AABB.y || f_pos.x > f_AABB.z || f_pos.y > f_AABB.w )
	   discard ;

	uvec4 temp = uvec4( gl_FragCoord.x, gl_FragCoord.y, u_voxelSize * gl_FragCoord.z, 0 ) ;
	uvec4 texcoord;
	if( f_axis == 1 )
	{
	    texcoord.x = u_voxelSize - temp.z;
		texcoord.z = temp.x;
		texcoord.y = temp.y;
		texcoord.w = 0;
	}
	else if( f_axis == 2 )
    {
	    texcoord.z = temp.y;
		texcoord.y = u_voxelSize-temp.z;
		texcoord.x = temp.x;
		texcoord.w = 0;
	}
	else
	    texcoord = temp;

	uint idx = atomicCounterIncrement( u_voxelFragCount );
	if( u_bStore == 1 )
	{
		vec4 diffColor = vec4(u_Color, 1);
		vec4 normal = vec4(normalize(f_normal) * 0.5 + 0.5, 1.0);

		imageStore(u_voxelListPos, int(idx), texcoord);
		imageStore(u_voxelListColor, int(idx), diffColor);
		imageStore(u_voxelListNormal, int(idx), normal);
	}
}