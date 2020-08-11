#pragma once
#include <iostream>
#include "opengl_common.h"
#include "world.h"
#include "automic_buffer.h"

class SVO_VCT
{
public:
	SVO_VCT();
	~SVO_VCT();

	void SparseVoxelize(World& world);

private:
	void BuildVoxelList(World& world);
	void BuildOctree();
	void AllocBrick();
	void WriteLeafNode();
	void SpreadLeafBrick();
	void BorderTransfer(int level);

	uint32_t m_voxelSize;
	uint32_t m_octreeLevel;
	uint32_t m_brickPoolDim;

	uint32_t m_numVoxelFrag;
	std::vector<uint32_t> m_startOfLevel;//the vector records the start index of nodes in each tree level
	std::vector<uint32_t> m_numOfLevel;//the vector records the number of nodes in each tree level

	FramebufferPtr m_voxel_FBO;
	AutomicBufferPtr m_automic_count;

	TextureBufferPtr m_voxel_list_pos;
	TextureBufferPtr m_voxel_list_color;
	TextureBufferPtr m_voxel_list_normal;

	TextureBufferPtr m_octree_node_idx;
	TextureBufferPtr m_octree_node_brick_idx;

	TextureBufferPtr m_octree_node_neigh_x_idx;
	TextureBufferPtr m_octree_node_neigh_y_idx;
	TextureBufferPtr m_octree_node_neigh_z_idx;

	Texture3DPtr m_octree_brick_color;
	Texture3DPtr m_octree_brick_normal;
	Texture3DPtr m_octree_brick_irradiance;
};