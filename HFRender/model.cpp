#include "model.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "helpers.h"
#include "config.h"

ModelData::ModelData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	m_vb = std::make_shared<VertexBuffer>(VertexBuffer::BufferType::VERTEX, vertices.data(), vertices.size() * sizeof(Vertex));
	m_ib = std::make_shared<VertexBuffer>(VertexBuffer::BufferType::INDEX, indices.data(), indices.size() * sizeof(uint32_t));
	m_stride = sizeof(Vertex);
    m_prim_count = indices.size() / 3;
    m_point_count = vertices.size();
}

ModelData::~ModelData()
{
	if (m_vao)
	{
		glDeleteVertexArrays(1, &m_vao);
		GL_CHECK_ERROR;
		m_vao = 0;
	}
}

void ModelData::FillRenderContext(RenderContext* const render_context)
{
	if (!m_graphic_resources_inited)
	{
		CreateGraphicResources();
		m_graphic_resources_inited = true;
	}

    render_context->SetPointCount(m_point_count);
	render_context->SetPrimCount(m_prim_count);
	render_context->SetIndexOffset((void*)m_index_offset);
	render_context->SetVao(m_vao);
}

void ModelData::CreateGraphicResources()
{
	glGenVertexArrays(1, &m_vao);
	GL_CHECK_ERROR;

	// 先绑定VAO
	glBindVertexArray(m_vao);
	GL_CHECK_ERROR;

	// 绑定VBO
	m_vb->Bind();

	// 绑定EBO
	m_ib->Bind();

	// 声明顶点数据格式
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);
	GL_CHECK_ERROR;

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, m_stride, (GLvoid*)offsetof(Vertex, Vertex::normal));
    GL_CHECK_ERROR;

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, m_stride, (GLvoid*)offsetof(Vertex, Vertex::uv));
    GL_CHECK_ERROR;

    //不能在解绑VAO之前解绑索引数组缓冲
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
    GL_CHECK_ERROR;
}

ModelEntity::ModelEntity(const std::string& path, const glm::mat4& transform, const MaterialPtr& material) :m_path(path), m_transform(transform), m_material(material)
{
	m_rc.SetTransform(transform);
    m_rc.SetMaterial(material);

	m_model_data = ModelLoader::Instance()->LoadModel(path);
	if (m_model_data)
	{
		m_model_data->FillRenderContext(&m_rc);
	}
}

ModelEntity::ModelEntity(const ModelDataPtr& model_data, const glm::mat4& transform, const MaterialPtr& material)
{
    m_rc.SetTransform(transform);
    m_rc.SetMaterial(material);

    m_model_data = model_data;
    m_model_data->FillRenderContext(&m_rc);
}

void ModelEntity::SetMaterial(const MaterialPtr& material)
{
	m_material = material;
	m_rc.SetMaterial(material);
}

void ModelEntity::SetTransform(const glm::mat4& transform)
{
    m_transform = transform;
    m_rc.SetTransform(m_transform);
}

void ModelEntity::CommitRenderContext(ViewContext& view_context)
{
	if (m_model_data && (m_flag & 0x00000001))
	{
		view_context.AddRenderContext(&m_rc);
	}
}

void ModelEntity::ClearModelData()
{
	m_model_data.reset();
	ModelLoader::Instance()->ClearCache(m_path);
}

void ModelEntity::SetRenderEnable(bool enable)
{
    if (enable)
    {
        m_flag |= 0x00000001;
    }
    else
    {
        m_flag &= (~0x00000001);
    }
}

void ModelEntity::UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param)
{
    if (m_material)
    {
        for (auto& it : param)
        {
            m_material->SetParam(it.first, it.second);
        }
        for (auto& it : texture_param)
        {
            m_material->SetTextureParam(it.first, it.second);
        }
        for (auto& it : image_param)
        {
            m_material->SetImageParam(it.first, it.second);
        }
    }
}

Volume::Volume(glm::vec3 start, float stride, uint32_t width, uint32_t height, uint32_t depth, const MaterialPtr& material)
    :m_start(start), m_stride(stride), m_width(width), m_height(height), m_depth(depth), m_material(material)
{
    std::string path = Config::Instance()->project_path + "resource/model/cube_2.obj";
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    if (!tinyobj::LoadObj(shapes, materials, err, path.c_str(), "") || shapes.size() == 0) 
    {
        std::cerr << "Failed to load object with path '" << path << "'. Error message:" << std::endl << err << std::endl;
        assert(0);
    }
    tinyobj::mesh_t mesh = shapes[0].mesh;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t count = 0;
    for (uint32_t x = 0;x < width;x++)
    {
        for (uint32_t y = 0;y < height;y++)
        {
            for (uint32_t z = 0;z < depth;z++)
            {
                glm::vec3 point = start + glm::vec3(x, y, z) * stride;
                glm::vec3 center = point + glm::vec3(0.5 * stride);
                uint32_t numPoint = mesh.positions.size() / 3;
                for (int i = 0; i < numPoint; i++)
                {
                    Vertex vertex;
                    vertex.position = point + glm::vec3(mesh.positions[i * 3], mesh.positions[i * 3 + 1], mesh.positions[i * 3 + 2]) * stride;
                    vertex.normal = center;
                    vertices.emplace_back(vertex);
                }
                for (int i = 0; i < mesh.indices.size(); i++)
                {
                    indices.push_back(count + mesh.indices[i]);
                }
                count += numPoint;
            }
        }
    }

    m_rc.SetTransform(glm::mat4(1));
    m_rc.SetMaterial(material);
    m_model_data = std::make_shared<ModelData>(vertices, indices);
    m_model_data->FillRenderContext(&m_rc);
}

void Volume::CommitRenderContext(ViewContext& view_context)
{
    if (m_model_data && (m_flag & 0x00000001))
    {
        view_context.AddRenderContext(&m_rc);
    }
}

void Volume::SetRenderEnable(bool enable)
{
    if (enable)
    {
        m_flag |= 0x00000001;
    }
    else
    {
        m_flag &= (~0x00000001);
    }
}

PointList::PointList(uint32_t pointNum, const MaterialPtr& material) :m_pointNum(pointNum)
{
    std::vector<Vertex> vertices;
    for (int i = 0; i < pointNum; i++)
    {
        Vertex vertex;
        vertex.position = glm::vec3(i);
        vertices.emplace_back(vertex);
    }

    m_rc.SetTransform(glm::mat4(1));
    m_rc.SetMaterial(material);
    m_rc.SetRenderMode(GL_POINTS);
    m_model_data = std::make_shared<ModelData>(vertices, std::vector<uint32_t>());
    m_model_data->FillRenderContext(&m_rc);
}

void PointList::CommitRenderContext(ViewContext& view_context)
{
    if (m_model_data && (m_flag & 0x00000001))
    {
        view_context.AddRenderContext(&m_rc);
    }
}

void PointList::SetRenderEnable(bool enable)
{
    if (enable)
    {
        m_flag |= 0x00000001;
    }
    else
    {
        m_flag &= (~0x00000001);
    }
}

Quad::Quad(const MaterialPtr& material)
{
    std::string path = Config::Instance()->project_path + "resource/model/quad_1.obj";
    m_model_data = ModelLoader::Instance()->LoadModel(path);
    m_model_data->FillRenderContext(&m_rc);

    m_rc.SetTransform(glm::mat4(1));
    m_rc.SetMaterial(material);
}

void Quad::CommitRenderContext(ViewContext& view_context)
{
    if (m_model_data && (m_flag & 0x00000001))
    {
        view_context.AddRenderContext(&m_rc);
    }
}

void Quad::SetRenderEnable(bool enable)
{
    if (enable)
    {
        m_flag |= 0x00000001;
    }
    else
    {
        m_flag &= (~0x00000001);
    }
}

ModelLoader* ModelLoader::s_inst = new ModelLoader();

ModelDataPtr ModelLoader::LoadModel(const std::string& path)
{
	auto it = m_model_cache.find(path);
	if (it != m_model_cache.end())
	{
		return it->second;
	}

    if (path.rfind(".obj") != std::string::npos)
    {
        std::vector<ModelDataPtr> models = LoadMesh(path);
        if (!models.empty())
        {
            m_model_cache[path] = models[0];
            return models[0];
        }
    }
	
	return ModelDataPtr();
}

std::vector<ModelDataPtr> ModelLoader::LoadModels(const std::string& path, const std::string& mtlPath)
{
    return LoadMesh(path, mtlPath);
}

void ModelLoader::ClearCache()
{
	m_model_cache.clear();
}

void ModelLoader::ClearCache(const std::string& path)
{
	if (m_is_enable_erase_data)
	{
		auto it = m_model_cache.find(path);
		if (it != m_model_cache.end() && it->second.unique())
		{
			m_model_cache.erase(it);
		}
	}
}

std::vector<ModelDataPtr> ModelLoader::LoadMesh(const std::string& path, const std::string& mtlPath)
{
    std::vector<ModelDataPtr> models;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    if (!tinyobj::LoadObj(shapes, materials, err, path.c_str(), mtlPath.c_str()) || shapes.size() == 0) {
        std::cerr << "Failed to load object with path '" << path << "'. Error message:" << std::endl << err << std::endl;
        return models;
    }

    std::vector<TinyobjMaterialPtr> materialPtrs;
    for (const auto& material : materials)
    {
        materialPtrs.emplace_back(std::make_shared<tinyobj::material_t>(material));
    }

    for (const auto& shape : shapes)
    {
        int numVertex = shape.mesh.positions.size() / 3;

        AABB aabb;
        std::vector<Vertex> vertices;
        vertices.resize(numVertex);
        for (int i = 0; i < numVertex; i++)
        {
            Vertex vertex;
            vertex.position = glm::vec3(shape.mesh.positions[i * 3], shape.mesh.positions[i * 3 + 1], shape.mesh.positions[i * 3 + 2]);
            if (shape.mesh.normals.size() > 0)
            {
                vertex.normal = glm::vec3(shape.mesh.normals[i * 3], shape.mesh.normals[i * 3 + 1], shape.mesh.normals[i * 3 + 2]);
            }
            if (shape.mesh.texcoords.size() > 0)
            {
                vertex.uv = glm::vec2(shape.mesh.texcoords[i * 2], shape.mesh.texcoords[i * 2 + 1]);
            }
            
            vertices[i] = vertex;
            aabb.Merge(vertex.position);
        }
        
        std::vector<uint32_t> indices(shape.mesh.indices);
        ModelDataPtr modelData = std::make_shared<ModelData>(vertices, indices);
        modelData->SetBoundingBox(aabb);
        if (!materials.empty() && shape.mesh.material_ids[0] >= 0)
        {
            modelData->SetMaterial(materialPtrs[shape.mesh.material_ids[0]]);
        }
        models.push_back(modelData);
    }

    return models;
}