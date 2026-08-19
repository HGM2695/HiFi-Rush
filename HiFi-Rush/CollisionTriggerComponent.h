#pragma once

#include "Component.h"
#include "Event.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class TriggerSystem;
	struct Collision3DEvent;

	class CollisionTriggerComponent final : public Component
	{
	public:
		CollisionTriggerComponent(const std::wstring& colliderId, const std::wstring& triggerId);

	protected:
		void OnInitialize() override;

	private:
		void HandleCollisionEnter(const Collision3DEvent& event);

	private:
		std::wstring			_colliderId{};
		std::wstring			_triggerId{};
		Collider3DComponent*	_collider = nullptr;
		TriggerSystem*			_triggerSystem = nullptr;
		EventConnection			_collisionEnterConnection{};
	};
}
