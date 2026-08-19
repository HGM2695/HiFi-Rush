#pragma once

#include "Component.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class CameraComponent;
	class CombatTargetComponent;
	class GameObject;
	class PhysicsSystem3D;
	class TransformComponent;

	struct PlayerTargetingDesc
	{
		float findRadius = 5.f;
		float releaseRadius = 6.5f;
		float closeTargetRadius = 1.5f;
		float viewAngle = 80.f;
		float directionWeight = 1.5f;
		float currentTargetBonus = 2.f;
	};

	class PlayerTargetingComponent final : public Component
	{
	public:
		PlayerTargetingComponent(const PhysicsSystem3D& physicsSystem, const CameraComponent& referenceCamera, PlayerTargetingDesc desc = {});

		GameObject*	AcquireTarget(const Vector3& direction = {});
		GameObject*	GetTarget() const;
		Vector3		GetTargetPosition() const;
		Vector3		GetTargetDirection() const;
		void		ClearTarget();

	protected:
		void	OnInitialize() override;

	private:
		Vector3 GetFindDirection(const Vector3& preferredDirection) const;
		bool	canTargeting(const CombatTargetComponent& target, const Vector3& findDirection, float maxDistance, bool isCheckTargetAngle) const;
		float	EvaluateTargetCost(const CombatTargetComponent& target, const Vector3& findDirection) const;
		void	SetTarget(CombatTargetComponent* target);

	private:
		const PhysicsSystem3D&	_physicsSystem;
		const CameraComponent*	_referenceCamera = nullptr;
		WeakGameObjectPtr		_referenceCameraOwner{};
		WeakGameObjectPtr		_target{};
		CombatTargetComponent*	_targetComponent = nullptr;
		TransformComponent*		_ownerTransform = nullptr;
		PlayerTargetingDesc		_desc{};
	};
}
