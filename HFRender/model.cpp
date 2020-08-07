#include "model.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "helpers.h"

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

void ModelEntity::SetMaterial(const MaterialPtr& material)
{
	m_material = material;
	m_rc.SetMaterial(material);
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
    std::vector<Vertex> vertices;
    for (uint32_t x = 0;x < width;x++)
    {
        for (uint32_t y = 0;y < height;y++)
        {
            for (uint32_t z = 0;z < depth;z++)
            {
                Vertex vertex;
                vertex.position = start + glm::vec3(x * stride, y * stride, z * stride);
                vertices.emplace_back(vertex);
            }
        }
    }

    m_rc.SetTransform(glm::mat4(1));
    m_rc.SetMaterial(material);
    m_rc.SetRenderMode(GL_POINTS);
    m_model_data = std::make_shared<ModelData>(vertices, std::vector<uint32_t>());
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
        ModelDataPtr model = LoadMesh(path);
        if (model)
        {
            m_model_cache[path] = model;
            return model;
        }
    }
	
	return ModelDataPtr();
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

ModelDataPtr ModelLoader::LoadMesh(const std::string& path)
{
    std::ifstream in(path, std::ios::in);
    if (in.fail())
    {
        std::cout << "obj file open failed: " << path << std::endl;
        return ModelDataPtr();
    }

    std::vector<glm::vec3> points;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::ivec3> ids;

    while (!in.eof())
    {
        std::string line;
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            glm::vec3 vec;
            iss >> vec[0] >> vec[1] >> vec[2];
            points.push_back(vec);
        }
        else if (!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            glm::vec3 vec;
            int i = 0;
            float v;
            while (iss >> v)
            {
                vec[i++] = v;
            }
            if (i == 2)
                vec[2] = 0;
            uvs.push_back(vec);
        }
        else if (!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            glm::vec3 vec;
            iss >> vec[0] >> vec[1] >> vec[2];
            normals.push_back(vec);
        }
        else if (!line.compare(0, 2, "f "))
        {
            iss >> trash;
            if (line.find("//") != std::string::npos)
            {
                for (size_t i = 0;i < 3;i++)
                {
                    glm::ivec3 id(0);
                    iss >> id.x >> trash >> trash >> id.z;
                    ids.push_back(id);
                }
            }
            else
            {
                int count = std::count(line.begin(), line.end(), '/');
                count /= 3;
                if (count <= 2)
                {
                    for (size_t i = 0;i < 3;i++)
                    {
                        glm::ivec3 id(0);
                        iss >> id.x;
                        for (size_t j = 1;j <= count;j++)
                        {
                            iss >> trash >> id[j];
                        }
                        ids.push_back(id);
                    }
                }
                else
                {
                    std::cout << "obj file is error: " << path << std::endl;
                    return ModelDataPtr();
                }
            }
        }
    }

    std::unordered_map<std::string, uint32_t> vtx_map;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (const auto& id : ids)
    {
        std::string key = std::to_string(id.x) + "/" + std::to_string(id.y) + "/" + std::to_string(id.z);
        auto it = vtx_map.find(key);
        if (it != vtx_map.end())
        {
            indices.push_back(it->second);
        }
        else
        {
            assert(id.x > 0);
            Vertex vertex;
            vertex.position = points[id.x - 1];
            if (id.y > 0)
            {
                vertex.uv = uvs[id.y - 1];
            }
            if (id.z > 0)
            {
                vertex.normal = normals[id.z - 1];
            }
            
            vertices.push_back(vertex);
            uint32_t cur_pos = vertices.size() - 1;
            vtx_map[key] = cur_pos;
            indices.push_back(cur_pos);
        }
    }

    return std::make_shared<ModelData>(vertices, indices);
}