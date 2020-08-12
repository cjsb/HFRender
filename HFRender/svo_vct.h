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
	void LightUpdate(World& world);
	void RenderOctree();

private:
	void BuildVoxelList(World& world);
	void BuildOctree();
	void AllocBrick();
	void WriteLeafNode();
	void SpreadLeafBrick(const Texture3DPtr& octree_brick);
	void BorderTransfer(int level, const Texture3DPtr& octree_brick);
	void Mipmap(int level, const Texture3DPtr& octree_brick, const glm::vec4& empty_color);
	void ShadowMap(World& world);
	void LightInjection(World& world);

	uint32_t m_shadowMapSize;
	uint32_t m_voxelSize;
	uint32_t m_octreeLevel;
	uint32_t m_brickPoolDim;

	uint32_t m_numVoxelFrag;
	std::vector<uint32_t> m_startOfLevel;//the vector records the start index of nodes in each tree level
	std::vector<uint32_t> m_numOfLevel;//the vector records the number of nodes in each tree level

	Texture2DPtr m_shadow_map;
	FramebufferPtr m_voxel_FBO;
	FramebufferPtr m_shadow_map_FBO;
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

typedef std::shared_ptr<SVO_VCT> SVO_VCTPtr;