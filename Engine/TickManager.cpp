#include "TickManager.h"
#include "Component.h"
#include "GameObject.h"
#include <algorithm>

namespace gm
{
	void TickManager::Register(Component& component)
	{
		auto& components = _componentsByTickGroup[TickGroupToIndex(component.GetTickGroup())];

		if (std::find(components.begin(), components.end(), &component) != components.end())
			return;

		components.push_back(&component);
	}

	void TickManager::Unregister(Component& component)
	{
		auto& components = _componentsByTickGroup[TickGroupToIndex(component.GetTickGroup())];
		auto iter = std::find(components.begin(), components.end(), &component);
		if (iter == components.end())
			return;

		components.erase(iter);
	}

	void TickManager::RegisterGameObject(GameObject& gameObject)
	{
		gameObject.ForEachComponent(
			[this](Component& component)
			{
				Register(component);
			});
	}

	void TickManager::UnregisterGameObject(GameObject& gameObject)
	{
		gameObject.ForEachComponent(
			[this](Component& component)
			{
				Unregister(component);
			}
		);
	}

	void TickManager::Tick(TickGroup group, float deltaTime)
	{
		for (Component* component : _componentsByTickGroup[TickGroupToIndex(group)])
		{
			if (component->IsEnabled() == false || component->GetOwner().IsPendingDestroy())
				continue;

			component->Tick(deltaTime);
		}
	}
}
