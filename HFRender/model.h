#pragma once
#include <string>
#include "vertex_buffer.h"
#include "render_context.h"
#include "tiny_obj_loader/tiny_obj_loader.h"
#include "aabb.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

typedef std::shared_ptr<tinyobj::material_t> TinyobjMaterialPtr;

class ModelData 
{
public:
	ModelData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	~ModelData();
	void FillRenderContext(RenderContext* const render_context);
	void SetMaterial(const TinyobjMaterialPtr& material) { m_material = material; }
	const TinyobjMaterialPtr& GetMaterial() { return m_material; }
	void SetBoundingBox(const AABB& bbox) { m_bbox = bbox; }
	const AABB& GetBoundingBox() { return m_bbox; }
private:
	void CreateGraphicResources();

	VertexBufferPtr m_vb;
	VertexBufferPtr m_ib;
	TinyobjMaterialPtr m_material;
	AABB m_bbox;

	GLuint m_vao = 0;
	uint32_t m_stride = 0;
	uint32_t m_point_count = 0;
	uint32_t m_prim_count = 0;
	size_t m_index_offset = 0;
	bool m_graphic_resources_inited = false;
};
typedef std::shared_ptr<ModelData> ModelDataPtr;

class IEntity
{
public:
	virtual ~IEntity() {}
	virtual void CommitRenderContext(ViewContext& view_context) = 0;
	virtual void SetRenderEnable(bool enable) = 0;
	virtual void UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param) = 0;
	virtual void SetMaterial(const MaterialPtr& material) = 0;
	virtual void SetTransform(const glm::mat4& transform) = 0;
};
typedef std::unique_ptr<IEntity> IEntityPtr;

class ModelEntity : public IEntity
{
public:
	ModelEntity(const std::string& path, const glm::mat4& transform, const MaterialPtr& material);
	ModelEntity(const ModelDataPtr& model_data, const glm::mat4& transform, const MaterialPtr& material);
	virtual void CommitRenderContext(ViewContext& view_context) override;
	virtual void SetRenderEnable(bool enable) override;
	virtual void UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param) override;

	virtual void SetMaterial(const MaterialPtr& material) override;
	virtual void SetTransform(const glm::mat4& transform) override;

	const std::string& GetPath() const { return m_path; }
	void SetModelData(const ModelDataPtr& model_data) { m_model_data = model_data; }
	void ClearModelData();
	const ModelDataPtr& GetModelData() { return m_model_data; }
protected:
	glm::mat4 m_transform;
	std::string m_path;
	ModelDataPtr m_model_data;
	RenderContext m_rc;
	MaterialPtr m_material;
	uint32_t m_flag = 0x00000001;
};
typedef std::unique_ptr<ModelEntity> ModelEntityPtr;

class Volume :public IEntity
{
public:
	Volume(glm::vec3 start, float stride, uint32_t width, uint32_t height, uint32_t depth, const MaterialPtr& material);
	virtual void CommitRenderContext(ViewContext& view_context) override;
	virtual void SetRenderEnable(bool enable) override;
	virtual void UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param)override {};
	virtual void SetMaterial(const MaterialPtr& material) override {};
	virtual void SetTransform(const glm::mat4& transform) override {};
private:
	glm::vec3 m_start;
	float m_stride;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	ModelDataPtr m_model_data;
	RenderContext m_rc;
	MaterialPtr m_material;
	uint32_t m_flag = 0x00000001;
};
typedef std::unique_ptr<Volume> VolumePtr;

class PointList :public IEntity
{
public:
	PointList(uint32_t pointNum, const MaterialPtr& material);
	virtual void CommitRenderContext(ViewContext& view_context) override;
	virtual void SetRenderEnable(bool enable) override;
	virtual void UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param)override {};
	virtual void SetMaterial(const MaterialPtr& material) override {};
	virtual void SetTransform(const glm::mat4& transform) override {};
private:
	uint32_t m_pointNum;
	ModelDataPtr m_model_data;
	RenderContext m_rc;
	MaterialPtr m_material;
	uint32_t m_flag = 0x00000001;
};
typedef std::unique_ptr<PointList> PointListPtr;

class Quad :public IEntity
{
public:
	Quad(const MaterialPtr& material);
	virtual void CommitRenderContext(ViewContext& view_context) override;
	virtual void SetRenderEnable(bool enable) override;
	virtual void UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param)override {};
	virtual void SetMaterial(const MaterialPtr& material) override {};
	virtual void SetTransform(const glm::mat4& transform) override {};
private:
	ModelDataPtr m_model_data;
	RenderContext m_rc;
	MaterialPtr m_material;
	uint32_t m_flag = 0x00000001;
};
typedef std::unique_ptr<Quad> QuadPtr;

class ModelLoader
{
public:
	static ModelLoader* Instance()
	{
		return s_inst;
	}

	ModelDataPtr LoadModel(const std::string& path);
	std::vector<ModelDataPtr> LoadModels(const std::string& path, const std::string& mtlPath = "");
	void ClearCache();
	void ClearCache(const std::string& path);
	void SetEnableErase(bool is_enable_erase_data) { m_is_enable_erase_data = is_enable_erase_data; }
protected:
	ModelLoader() {}
	std::vector<ModelDataPtr> LoadMesh(const std::string& path, const std::string& mtlPath = "");

	std::unordered_map<std::string, ModelDataPtr> m_model_cache;
	bool m_is_enable_erase_data = false;

	static ModelLoader* s_inst;
};

