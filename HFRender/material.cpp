#include <typeindex>
#include "material.h"
#include "config.h"
#include "camera.h"

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
	}

	for (const auto& it : m_texture_param)
	{
		m_shader->SetTexture(it.first, it.second);
	}

	for (const auto& it : m_image_param)
	{
		m_shader->SetImage(it.first, it.second);
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
	ShaderPtr shader = ShaderLoader::Instance()->LoadShader(vs_path, fs_path, gs_path);
	return std::make_shared<Material>(std::move(shader), std::move(params));
}

std::shared_ptr<Material> Material::CreateMaterial(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path,
	ParamTable&& params, TextureParamTable&& texture_param)
{
	ShaderPtr shader = ShaderLoader::Instance()->LoadShader(vs_path, fs_path, gs_path);
	return std::make_shared<Material>(std::move(shader), std::move(params), std::move(texture_param));
}

std::shared_ptr<Material> Material::CreateMaterial(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path,
	ParamTable&& params, TextureParamTable&& texture_param, TextureParamTable&& image_param)
{
	ShaderPtr shader = ShaderLoader::Instance()->LoadShader(vs_path, fs_path, gs_path);
	return std::make_shared<Material>(std::move(shader), std::move(params), std::move(texture_param), std::move(image_param));
}

std::shared_ptr<Material> Material::GetVCTDiffuse(const Texture3DPtr& texture3D, glm::vec3 color)
{
	ParamTable params = {
		{"voxel_size", 2.f / Config::Instance()->voxelSize},
		{"material.diffuseColor",color},
		{"material.specularColor",color},
		{"material.diffuseReflectivity",1.f},
		{"material.specularReflectivity",0.f},
		{"material.specularDiffusion",0.f},
		{"material.emissivity",0.0f},
		{"material.transparency",0.0f},
		{"material.refractiveIndex",1.0f},
		{"settings.indirectSpecularLight", false},
		{"settings.indirectDiffuseLight", true},
		{"settings.directLight", true},
		{"settings.shadows", true},
		{"pointLights[0].position",glm::vec3(0, 0.9, 0)},
		{"pointLights[0].color",glm::vec3(1)},
		{"numberOfLights",1}
	};
	TextureParamTable texture_param = {
		{"texture3D", texture3D}
	};
	MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/voxel_cone_tracing.vert",
		Config::Instance()->project_path + "shader/voxel_cone_tracing.frag", "", std::move(params), std::move(texture_param));
	return material;
}

std::shared_ptr<Material> Material::GetVCTSpecular(const Texture3DPtr& texture3D, glm::vec3 color)
{
	ParamTable params = {
		{"voxel_size", 2.f / Config::Instance()->voxelSize},
		{"material.diffuseColor",color},
		{"material.specularColor",color},
		{"material.diffuseReflectivity",0.5f},
		{"material.specularReflectivity",0.5f},
		{"material.specularDiffusion",0.5f},
		{"material.emissivity",0.0f},
		{"material.transparency",0.0f},
		{"material.refractiveIndex",1.0f},
		{"settings.indirectSpecularLight", true},
		{"settings.indirectDiffuseLight", true},
		{"settings.directLight", true},
		{"settings.shadows", true},
		{"pointLights[0].position",glm::vec3(0, 0.9, 0)},
		{"pointLights[0].color",glm::vec3(1)},
		{"numberOfLights",1}
	};
	TextureParamTable texture_param = {
		{"texture3D", texture3D}
	};
	MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/voxel_cone_tracing.vert",
		Config::Instance()->project_path + "shader/voxel_cone_tracing.frag", "", std::move(params), std::move(texture_param));
	return material;
}

std::shared_ptr<Material> Material::GetVoxelMaterial(const Texture3DPtr& texture3D, glm::vec3 color)
{
	ParamTable params = {
		{"material.diffuseColor",color},
		{"material.specularColor",color},
		{"material.diffuseReflectivity", 0.5f},
		{"material.specularReflectivity",0.5f},
		{"material.emissivity",0.0f},
		{"material.transparency",0.0f},
		{"pointLights[0].position",glm::vec3(0, 0.9, 0)},
		{"pointLights[0].color",glm::vec3(1)},
		{"numberOfLights",1}
	};
	TextureParamTable image_param = {
		{"texture3D", texture3D}
	};
	MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/voxelization.vert",
		Config::Instance()->project_path + "shader/voxelization.frag",
		Config::Instance()->project_path + "shader/voxelization.geom", std::move(params), {}, std::move(image_param));
	return material;
}

std::shared_ptr<Material> Material::GetVoxelListMaterial(glm::vec3 color)
{
	glm::mat4 orth = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	Camera camera;
	camera.SetPosition(glm::vec3(0));

	camera.SetForward(glm::vec3(-1, 0, 0));
	glm::mat4 vpX = orth * camera.GetViewMatrix();

	camera.SetForward(glm::vec3(0, -1, 0));
	glm::mat4 vpY = orth * camera.GetViewMatrix();

	camera.SetForward(glm::vec3(0, 0, -1));
	glm::mat4 vpZ = orth * camera.GetViewMatrix();

	ParamTable params = {
		{"u_VPx", vpX},
		{"u_VPy", vpY},
		{"u_VPz", vpZ},
		{"u_voxelSize", Config::Instance()->voxelSize},
		{"u_Color", color},
		{"u_bStore", 0}
	};
	MaterialPtr material = Material::CreateMaterial(Config::Instance()->project_path + "shader/SparseVoxelOctree/voxelize_list.vert",
		Config::Instance()->project_path + "shader/SparseVoxelOctree/voxelize_list.frag",
		Config::Instance()->project_path + "shader/SparseVoxelOctree/voxelize_list.geom", std::move(params));
	return material;
}