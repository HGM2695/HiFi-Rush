#pragma once

#include "CombatTypes.h"
#include "Component.h"

namespace gm
{
	class Collider3DComponent;
	class HealthComponent;
	class HitBoxComponent;

	class HurtBoxComponent final : public Component
	{
	friend class HitBoxComponent;

	public:
		explicit HurtBoxComponent(Collider3DComponent& collider);

		Collider3DComponent&		GetCollider() { return _collider; }
		const Collider3DComponent&	GetCollider() const { return _collider; }

		EventPublisher<HurtBoxComponent, HitEvent> OnHurt;

	protected:
		void OnInitialize() override;

	private:
		DamageResult ReceiveHit(const HitEvent& event);

		Collider3DComponent&	_collider;
		HealthComponent*		_health = nullptr;
	};
}
