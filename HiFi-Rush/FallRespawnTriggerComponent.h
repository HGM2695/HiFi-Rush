#pragma once

#include "Component.h"
#include "Event.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class GameplayScene;
	struct Collision3DEvent;

	class FallRespawnTriggerComponent final : public Component
	{
	public:
		FallRespawnTriggerComponent(const std::wstring& colliderId, int32 damage);

	protected:
		void OnInitialize() override;

	private:
		void HandleCollisionEnter(const Collision3DEvent& event);

	private:
		std::wstring			_colliderId{};
		int32					_damage = 50;
		Collider3DComponent*	_collider = nullptr;
		GameplayScene*			_gameplayScene = nullptr;
		EventConnection			_collisionEnterConnection{};
	};
}
