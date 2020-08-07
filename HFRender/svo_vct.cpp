#include "svo_vct.h"
#include "config.h"

SVO_VCT::SVO_VCT()
{
	m_voxel_size = Config::Instance()->voxelSize;

	m_voxel_list_count = std::make_shared<AutomicBuffer>(0);

	m_voxel_FBO = std::make_shared<Framebuffer>();
	m_voxel_FBO->AttachColorBuffer(std::make_unique<RenderSurface>(m_voxel_size, m_voxel_size, GL_RGB));
}

SVO_VCT::~SVO_VCT()
{

}

void SVO_VCT::SparseVoxelize(World& world)
{
	BuildVoxelList(world);


}

void SVO_VCT::BuildVoxelList(World& world)
{
	m_voxel_list_count->BindBase(0);

	ViewContext voxel_vc;
	voxel_vc.SetColorMask(glm::bvec4(false));
	voxel_vc.SetDepthStates(false, false, GL_LESS);
	voxel_vc.SetCullFace(false, GL_BACK);
	voxel_vc.SetBlend(false);
	voxel_vc.SetFramebuffer(m_voxel_FBO);

	glViewport(0, 0, m_voxel_size, m_voxel_size);
	world.CommitRenderContext(voxel_vc);

	// Obtain number of voxel fragments
	voxel_vc.FlushRenderContext(false);
	m_voxel_list_count->Sync();
	m_numVoxelFrag = m_voxel_list_count->GetVal();
	m_voxel_list_count->SetVal(0);
	std::cout << "Number of Entries in Voxel Fragment List: " << m_numVoxelFrag << std::endl;

	// Create buffers for voxel fragment list
	m_voxel_list_pos = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_R32UI);
	m_voxel_list_pos->SetUnit(0);
	m_voxel_list_pos->SetInternalFormat(GL_RGB10_A2UI);
	m_voxel_list_color = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_RGBA8);
	m_voxel_list_color->SetUnit(1);
	m_voxel_list_normal = std::make_shared<TextureBuffer>(nullptr, sizeof(GLuint) * m_numVoxelFrag, GL_RGBA8);
	m_voxel_list_normal->SetUnit(2);

	ParamTable param = {
		{"u_bStore", 1}
	};
	TextureParamTable image_param = {
		{"u_voxelListPos", m_voxel_list_pos},
		{"u_voxelListColor", m_voxel_list_color},
		{"u_voxelListNormal", m_voxel_list_normal},
	};
	world.UpdateMaterialParam(param, {}, image_param);

	//Voxelize the scene again, this time store the data in the voxel fragment list
	voxel_vc.FlushRenderContext(false);
	m_voxel_list_pos->SyncImage();
}

void SVO_VCT::FlagNode()
{

}