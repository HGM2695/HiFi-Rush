#pragma once

#include "CombatTypes.h"
#include "GameObject.h"

#include <functional>
#include <optional>
#include <string>

namespace gm
{
	class Collider3DComponent;
	class HitBoxComponent;

	struct TemporaryHitBoxDesc
	{
		Matrix world = Matrix::Identity;
		std::wstring colliderId{};
		CollisionLayer collisionLayer = DefaultCollisionLayer;
		CollisionMask collisionMask = AllCollisionLayers;
		DamageInfo damageInfo{};
		float rehitInterval = 0.f;
		float activationDelayBeats = 0.f;
		float lifetime = 0.f;
		std::function<void()> onActivated{};
		std::function<void(const HitEvent&)> onHit{};
		std::function<bool(const HurtBoxComponent&)> hitCondition{};
	};

	struct TemporaryBoxHitBoxDesc : TemporaryHitBoxDesc
	{
		Vector3 localCenter{};
		Vector3 size{ 1.f, 1.f, 1.f };
	};

	struct TemporarySphereHitBoxDesc : TemporaryHitBoxDesc
	{
		Vector3 localCenter{};
		float radius = 0.5f;
	};

	class TemporaryHitBoxObject final : public GameObject
	{
	public:
		explicit TemporaryHitBoxObject(const TemporaryBoxHitBoxDesc& desc);
		explicit TemporaryHitBoxObject(const TemporarySphereHitBoxDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void Activate();
		void ConfigureHitBox(Collider3DComponent& collider, const TemporaryHitBoxDesc& desc);

		std::function<void()> _onActivated{};
		std::function<void(const HitEvent&)> _onHit{};
		EventConnection _hitConnection{};
		HitBoxComponent* _hitBox = nullptr;
		std::optional<float> _activationBeat{};
		float _lifetime = 0.f;
		float _elapsedTime = 0.f;
		float _activationDelayBeats = 0.f;
		bool _hasActivated = false;
	};
}
