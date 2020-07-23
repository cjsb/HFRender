#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include "shader.h"

typedef std::unordered_map<std::string, std::any> ParamTable;

class Material
{
public:
	Material() = default;
	Material(ShaderPtr&& shader, ParamTable&& params) :
		m_shader(std::move(shader)),
		m_params(std::move(params))
	{}
	void SetShader(const std::shared_ptr<Shader>& shader) { m_shader = shader; }
	const std::shared_ptr<Shader>& GetShader() const { return m_shader; }
	void SetParam(const std::string& name, std::any value) { m_params[name] = value; }
	void Apply();

	static std::shared_ptr<Material> GetDefaultMaterial();
	static std::shared_ptr<Material> CreateMaterial(const std::string& vs_path, const std::string& fs_path, ParamTable&& params);
protected:
	std::shared_ptr<Shader> m_shader;
	ParamTable m_params;
};
typedef std::shared_ptr<Material> MaterialPtr;