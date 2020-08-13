#version 450 core
layout(rgb10_a2ui) uniform uimageBuffer u_voxelListPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float u_pointSize;
uniform int u_voxelDim;

flat out uvec4 frag_loc;

void main(){
	int frag_idx = int(gl_VertexID);
	uvec4 loc = imageLoad(u_voxelListPos, frag_idx);
	vec4 pos = vec4(loc)/float(u_voxelDim);
	pos = pos*2-1;
	frag_loc = loc;

	gl_Position = projection * view * model * vec4(pos.xyz, 1);
	gl_PointSize = u_pointSize; 
}