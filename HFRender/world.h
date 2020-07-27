#pragma once
#include "opengl_common.h"
#include "model.h"

class World
{
public:
	World();
	void CommitRenderContext(ViewContext& view_context);
	void AddModelEntity(const std::string& path, const glm::mat4& transform, const std::string& name);

private:
	std::unordered_map<std::string, IEntityPtr> m_entities;
};