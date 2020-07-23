#include "model.h"
#include <sstream>
#include <fstream>
#include <unordered_map>

ModelData::ModelData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	m_vb = std::make_shared<VertexBuffer>(VertexBuffer::BufferType::VERTEX, vertices.data(), vertices.size() * sizeof(Vertex));
	m_ib = std::make_shared<VertexBuffer>(VertexBuffer::BufferType::INDEX, indices.data(), indices.size() * sizeof(uint32_t));
	m_stride = sizeof(Vertex);
	m_prim_count = (uint32_t)indices.size();
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

void ModelData::FillRenderContext(const RenderContextPtr& render_context)
{
	if (!m_graphic_resources_inited)
	{
		CreateGraphicResources();
		m_graphic_resources_inited = true;
	}

	render_context->SetPrimCount(m_prim_count);
	render_context->SetIndexOffset((void*)m_index_offset);
	render_context->SetVao(m_vao);
}

void ModelData::CreateGraphicResources()
{
	glGenVertexArrays(1, &m_vao);
	GL_CHECK_ERROR;

	// �Ȱ�VAO
	glBindVertexArray(m_vao);
	GL_CHECK_ERROR;

	// ��VBO
	m_vb->Bind();

	// ��EBO
	m_ib->Bind();

	// �����������ݸ�ʽ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);
	GL_CHECK_ERROR;

	glEnableVertexAttribArray(0);
	GL_CHECK_ERROR;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL_CHECK_ERROR;

	glBindVertexArray(0);
	GL_CHECK_ERROR;
}

ModelEntity::ModelEntity(const glm::mat4& transform, const std::string& path) :m_transform(transform), m_path(path)
{
	m_rc = std::make_shared<RenderContext>();
	m_rc->SetTransform(m_transform);
	m_rc->SetMaterial(m_material);

	m_model_data = ModelLoader::Instance()->LoadModel(path);
	if (m_model_data)
	{
		m_model_data->FillRenderContext(m_rc);
	}
}

void ModelEntity::SetMaterial(const MaterialPtr& material)
{
	m_material = material;
	m_rc->SetMaterial(material);
}

void ModelEntity::CommitRenderContext(ViewContext& view_context)
{
	if (m_model_data)
	{
		view_context.AddRenderContext(rc);
	}
}

void ModelEntity::ClearModelData()
{
	m_model_data.reset();
	ModelLoader::Instance()->ClearCache(m_path);
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
    std::ifstream in(file, std::ios::in);
    if (in.fail()) return ModelDataPtr();

    std::vector<glm::vec3> points;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::ivec3> ids;
    int count = -1;
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
            if (count < 0)
            {
                count = 0;
                for (auto iter : line)
                {
                    if (iter == '/')
                        count++;
                }
                count /= 3;
                assert(count < 3);
            }

            iss >> trash;
            for (int i = 0; i < 3; i++)
            {
                glm::ivec3 id(0);

                iss >> id.x;
                if (count > 0)
                {
                    iss >> trash >> id.y;
                }
                if (count > 1)
                {
                    iss >> trash >> id.z;
                }

                ids.push_back(id);
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
            indices.push_back(it.second);
        }
        else
        {
            assert(id.x > 0);
            Vertex vertex;
            vertex.position = points[id.x - 1];
            if (id.y > 0)
            {
                vertex.normal = normals[id.y - 1];
            }
            if (id.z > 0)
            {
                vertex.uv = uvs[id.z - 1];
            }
            
            vertices.push_back(vertex);
            uint32_t cur_pos = vertices.size() - 1;
            vtx_map[key] = cur_pos;
            indices.push_back(cur_pos);
        }
    }

    return std::make_shared<ModelData>(vertices, indices);
}