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

void World::CommitRenderContext(ViewContext& view_context, const std::string& name)
{
	auto it = m_entities.find(name);
	if (it != m_entities.end())
	{
		it->second->CommitRenderContext(view_context);
	}
}

void World::AddModelEntity(const std::string& path, const glm::mat4& transform, const std::string& name, const MaterialPtr& material)
{
	ModelEntityPtr model_entity = std::make_unique<ModelEntity>(path, transform, material);
	m_entities.emplace(name, std::move(model_entity));
}

void World::AddModelEntity(const ModelDataPtr& modelData, const glm::mat4& transform, const std::string& name, const MaterialPtr& material)
{
	ModelEntityPtr model_entity = std::make_unique<ModelEntity>(modelData, transform, material);
	m_entities.emplace(name, std::move(model_entity));
}

void World::AddEntity(const std::string& name, IEntityPtr&& entity)
{
	m_entities.emplace(name, std::move(entity));
}

void World::SetModelRenderEnable(const std::string& name, bool enable)
{
	auto it = m_entities.find(name);
	if (it != m_entities.end())
	{
		it->second->SetRenderEnable(enable);
	}
}

void World::UpdateMaterialParam(const ParamTable& param, const TextureParamTable& texture_param, const TextureParamTable& image_param)
{
	for (auto& it : m_entities)
	{
		it.second->UpdateMaterialParam(param, texture_param, image_param);
	}
}

void World::SetMaterial(const MaterialPtr& material)
{
	for (auto& it : m_entities)
	{
		it.second->SetMaterial(material);
	}
}

void World::UpdateTransform(const glm::mat4& transform)
{
	for (auto& it : m_entities)
	{
		it.second->SetTransform(transform);
	}
}
