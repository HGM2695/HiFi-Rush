#pragma once

#include "CombatTypes.h"
#include "Component.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class HealthComponent;
	class HitBoxComponent;

	class HurtBoxComponent final : public Component
	{
	friend class HitBoxComponent;

	public:
		explicit HurtBoxComponent(const std::wstring& colliderId);

		Collider3DComponent&		GetCollider() { return *_collider; }
		const Collider3DComponent&	GetCollider() const { return *_collider; }
		const std::wstring&		GetColliderId() const { return _colliderId; }

		EventPublisher<HurtBoxComponent, HitEvent> OnHurt;

	protected:
		void OnInitialize() override;

	private:
		DamageResult ReceiveHit(const HitEvent& event);

		std::wstring		_colliderId{};
		Collider3DComponent*	_collider = nullptr;
		HealthComponent*		_health = nullptr;
	};
}
