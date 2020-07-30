#pragma once
#include "opengl_common.h"
#include "model.h"

class World
{
public:
	World();
	void CommitRenderContext(ViewContext& view_context);
	void CommitRenderContext(ViewContext& view_context, const std::string& name);
	void AddModelEntity(const std::string& path, const glm::mat4& transform, const std::string& name, const MaterialPtr& material);
	void AddEntity(const std::string& name, IEntityPtr&& entity);
	void SetModelRenderEnable(const std::string& name, bool enable);
protected:
	std::unordered_map<std::string, IEntityPtr> m_entities;
};