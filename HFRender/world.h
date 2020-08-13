#pragma once
#include "opengl_common.h"
#include "model.h"
#include "light.h"

class World
{
public:
	World();
	void CommitRenderContext(ViewContext& view_context);
	void CommitRenderContext(ViewContext& view_context, const std::string& name);
	void AddModelEntity(const std::string& path, const glm::mat4& transform, const std::string& name, const MaterialPtr& material);
	void AddModelEntity(const ModelDataPtr& modelData, const glm::mat4& transform, const std::string& name, const MaterialPtr& material);
	void AddEntity(const std::string& name, IEntityPtr&& entity);
	void SetModelRenderEnable(const std::string& name, bool enable);
	void UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param);
	void SetMaterial(const MaterialPtr& material);
	const std::vector<DirectionLight>& GetLights() { return m_lights; }
	void AddLight(const DirectionLight& light) { m_lights.push_back(light); }
	void UpdateTransform(const glm::mat4& transform);
	const std::unordered_map<std::string, IEntityPtr>& GetEntities() { return m_entities; }
protected:
	std::unordered_map<std::string, IEntityPtr> m_entities;
	std::vector<DirectionLight> m_lights;
};