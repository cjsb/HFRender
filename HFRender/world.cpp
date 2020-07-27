#include "world.h"
#include "config.h"

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

void World::AddModelEntity(const std::string& path, const glm::mat4& transform, const std::string& name)
{
	MaterialPtr default_material = Material::GetDefaultMaterial();
	ModelEntityPtr model_entity = std::make_unique<ModelEntity>(path, transform, default_material);
	m_entities.emplace(name, std::move(model_entity));
}