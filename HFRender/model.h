#pragma once
#include <string>
#include "vertex_buffer.h"
#include "render_context.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

class ModelData 
{
public:
	ModelData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	~ModelData();
	void FillRenderContext(RenderContext* const render_context);

private:
	void CreateGraphicResources();

	VertexBufferPtr m_vb;
	VertexBufferPtr m_ib;
	GLuint m_vao = 0;
	uint32_t m_stride = 0;
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
};
typedef std::unique_ptr<IEntity> IEntityPtr;

class ModelEntity : public IEntity
{
public:
	ModelEntity(const std::string& path, const glm::mat4& transform, const MaterialPtr& material);
	virtual void CommitRenderContext(ViewContext& view_context) override;
	virtual void SetRenderEnable(bool enable) override;

	void SetMaterial(const MaterialPtr& material);
	const std::string& GetPath() const { return m_path; }
	void SetModelData(const ModelDataPtr& model_data) { m_model_data = model_data; }
	void ClearModelData();
	
protected:
	glm::mat4 m_transform;
	std::string m_path;
	ModelDataPtr m_model_data;
	RenderContext m_rc;
	MaterialPtr m_material;
	uint32_t m_flag = 0x00000001;
};
typedef std::unique_ptr<ModelEntity> ModelEntityPtr;

class ModelLoader
{
public:
	static ModelLoader* Instance()
	{
		return s_inst;
	}

	ModelDataPtr LoadModel(const std::string& path);
	void ClearCache();
	void ClearCache(const std::string& path);
	void SetEnableErase(bool is_enable_erase_data) { m_is_enable_erase_data = is_enable_erase_data; }
protected:
	ModelLoader() {}
	ModelDataPtr LoadMesh(const std::string& path);

	std::unordered_map<std::string, ModelDataPtr> m_model_cache;
	bool m_is_enable_erase_data = false;

	static ModelLoader* s_inst;
};