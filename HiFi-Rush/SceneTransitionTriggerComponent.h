#pragma once

#include "Component.h"
#include "Event.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	struct Collision3DEvent;

	class SceneTransitionTriggerComponent final : public Component
	{
	public:
		SceneTransitionTriggerComponent(const std::wstring& colliderId, const std::wstring& targetSceneName);

	protected:
		void OnInitialize() override;

	private:
		void HandleCollisionEnter(const Collision3DEvent& event);

		std::wstring			_colliderId{};
		std::wstring			_targetSceneName{};
		Collider3DComponent*	_collider = nullptr;
		EventConnection			_collisionEnterConnection{};
		bool					_hasTriggered = false;
	};
}
