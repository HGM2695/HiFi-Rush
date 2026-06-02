#pragma once

#include "EngineCore.h"
#include "TickGroup.h"
#include <array>
#include <vector>

namespace gm
{
	class Component;
	class GameObject;

	class TickManager
	{
	public:
		void	Tick(TickGroup group, float deltaTime);

		void	Register(Component& component);
		void	Unregister(Component& component);
		void	RegisterGameObject(GameObject& gameObject);
		void	UnregisterGameObject(GameObject& gameObject);

	private:
		std::array<std::vector<Component*>, TickGroupCount>		_componentsByTickGroup{};
	};
}
