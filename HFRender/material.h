#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include "shader.h"

typedef std::unordered_map<std::string, std::any> ParamTable;
typedef std::unordered_map<std::string, ITexturePtr> TextureParamTable;

class Material
{
public:
	Material() = default;
	Material(ShaderPtr&& shader, ParamTable&& params) :
		m_shader(std::move(shader)),
		m_params(std::move(params))
	{}
	Material(ShaderPtr&& shader, ParamTable&& params, TextureParamTable&& texture_param) :
		m_shader(std::move(shader)),
		m_params(std::move(params)),
		m_texture_param(std::move(texture_param))
	{}
	Material(ShaderPtr&& shader, ParamTable&& params, TextureParamTable&& texture_param, TextureParamTable&& image_param) :
		m_shader(std::move(shader)),
		m_params(std::move(params)),
		m_texture_param(std::move(texture_param)),
		m_image_param(std::move(image_param))
	{}
	void SetShader(const ShaderPtr& shader) { m_shader = shader; }
	const ShaderPtr& GetShader() const { return m_shader; }
	void SetParam(const std::string& name, std::any value) { m_params[name] = value; }
	void SetTextureParam(const std::string& name, const ITexturePtr& texture) { m_texture_param[name] = texture; }
	void SetImageParam(const std::string& name, const ITexturePtr& texture) { m_image_param[name] = texture; }
	void Apply();

	static std::shared_ptr<Material> GetDefaultMaterial();
	static std::shared_ptr<Material> GetVCTDiffuse(const Texture3DPtr& texture3D, glm::vec3 color);
	static std::shared_ptr<Material> GetVCTSpecular(const Texture3DPtr& texture3D, glm::vec3 color);
	static std::shared_ptr<Material> GetVoxelMaterial(const Texture3DPtr& texture3D, glm::vec3 color);
	static std::shared_ptr<Material> GetVoxelListMaterial(glm::vec3 color);

	static std::shared_ptr<Material> CreateMaterial(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path, ParamTable&& params);
	static std::shared_ptr<Material> CreateMaterial(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path,
		ParamTable&& params, TextureParamTable&& texture_param);
	static std::shared_ptr<Material> CreateMaterial(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path,
		ParamTable&& params, TextureParamTable&& texture_param, TextureParamTable&& image_param);
protected:
	ShaderPtr m_shader;
	ParamTable m_params;
	TextureParamTable m_texture_param;
	TextureParamTable m_image_param;
};
typedef std::shared_ptr<Material> MaterialPtr;