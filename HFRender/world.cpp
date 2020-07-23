#include "world.h"

World::World()
{

}

void World::CommitRenderContext(ViewContext& view_context)
{
	for (auto& it: m_entities)
	{
		it.second->CommitRenderContext(view_context);
	}
}

void World::AddModelEntity(const std::string& path,const glm::mat4 transform, const std::string& name)
{
	ModelEntityPtr model_entity = std::make_unique<ModelEntity>(transform, path);
	m_entities.emplace(std::move(name), std::move(model_entity));
}