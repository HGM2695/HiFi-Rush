#pragma once

#include "Component.h"
#include "Event.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class GameplayScene;
	struct Collision3DEvent;

	class RespawnPointTriggerComponent final : public Component
	{
	public:
		RespawnPointTriggerComponent(const std::wstring& colliderId, const Vector3& respawnPosition, float respawnRotationY);

	protected:
		void OnInitialize() override;

	private:
		void HandleCollisionEnter(const Collision3DEvent& event);

	private:
		std::wstring			_colliderId{};
		Vector3					_respawnPosition{};
		float					_respawnRotationY = 0.f;
		Collider3DComponent*	_collider = nullptr;
		GameplayScene*			_gameplayScene = nullptr;
		EventConnection			_collisionEnterConnection{};
	};
}
