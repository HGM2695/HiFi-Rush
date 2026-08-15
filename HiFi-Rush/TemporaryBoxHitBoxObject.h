#pragma once

#include "CombatTypes.h"
#include "GameObject.h"

#include <string>

namespace gm
{
	class HitBoxComponent;

	struct TemporaryBoxHitBoxDesc
	{
		Matrix			world = Matrix::Identity;
		std::wstring	colliderId{};
		Vector3			localCenter{};
		Vector3			size{ 1.f, 1.f, 1.f };
		CollisionLayer	collisionLayer = DefaultCollisionLayer;
		CollisionMask	collisionMask = AllCollisionLayers;
		DamageInfo		damageInfo{};
		float			lifetime = 0.f;
	};

	class TemporaryHitBoxObject final : public GameObject
	{
	public:
		explicit TemporaryHitBoxObject(const TemporaryBoxHitBoxDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		HitBoxComponent*	_hitBox = nullptr;
		float				_lifetime = 0.f;
		float				_elapsedTime = 0.f;
	};
}
