#include <typeindex>
#include "material.h"
#include "config.h"

void Material::Apply()
{
	m_shader->Use();
	for (const auto& it : m_params)
	{
		const std::any& value = it.second;
		std::type_index ti = std::type_index(value.type());

		if (ti == std::type_index(typeid(glm::vec4)))
		{
			m_shader->SetVector4f(it.first, std::any_cast<glm::vec4>(value));
		}
		else if (ti == std::type_index(typeid(glm::vec3)))
		{
			m_shader->SetVector3f(it.first, std::any_cast<glm::vec3>(value));
		}
		else if (ti == std::type_index(typeid(glm::vec2)))
		{
			m_shader->SetVector2f(it.first, std::any_cast<glm::vec2>(value));
		}
		else if (ti == std::type_index(typeid(float)))
		{
			m_shader->SetFloat(it.first, std::any_cast<float>(value));
		}
		else if (ti == std::type_index(typeid(glm::mat4)))
		{
			m_shader->SetMatrix4(it.first, std::any_cast<glm::mat4>(value));
		}
		else if (ti == std::type_index(typeid(glm::mat3)))
		{
			m_shader->SetMatrix3(it.first, std::any_cast<glm::mat3>(value));
		}
		else if (ti == std::type_index(typeid(glm::mat2)))
		{
			m_shader->SetMatrix2(it.first, std::any_cast<glm::mat2>(value));
		}
		else if (ti == std::type_index(typeid(int)))
		{
			m_shader->SetInteger(it.first, std::any_cast<int>(value));
		}
		else if (ti == std::type_index(typeid(bool)))
		{
			m_shader->SetBool(it.first, std::any_cast<bool>(value));
		}
		else if (ti == std::type_index(typeid(ITexturePtr)))
		{
			ITexturePtr texture = std::any_cast<ITexturePtr>(value);
			m_shader->SetTexture(it.first, texture);
		}
	}
}

std::shared_ptr<Material> Material::GetDefaultMaterial()
{
	ParamTable params = { {"albedo", glm::vec4(0.f, 1.f, 0.f, 1.f)} };
	return Material::CreateMaterial(Config::Instance()->project_path + "shader/default.vs",
		Config::Instance()->project_path + "shader/default.fs", "", std::move(params));
}

std::shared_ptr<Material> Material::CreateMaterial(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path, ParamTable&& params)
{
	ShaderPtr shader = std::make_shared<Shader>();
	if (!shader->Init(vs_path, fs_path, gs_path))
	{
		return std::shared_ptr<Material>();
	}

	return std::make_shared<Material>(std::move(shader), std::move(params));
}
