#pragma once

#include "Component.h"

namespace gm
{
	enum class RigidbodyPositionConstraint : uint8
	{
		None = 0,
		FreezeX = 1 << 0,
		FreezeY = 1 << 1,
		FreezeZ = 1 << 2,
	};

	constexpr RigidbodyPositionConstraint operator|(RigidbodyPositionConstraint lhs, RigidbodyPositionConstraint rhs)
	{
		return static_cast<RigidbodyPositionConstraint>(static_cast<uint8>(lhs) | static_cast<uint8>(rhs));
	}

	class Rigidbody3DComponent : public Component
	{
	friend class PhysicsSystem3D;

	public:
		void					SetVelocity(const Vector3& velocity) { _velocity = ApplyPositionConstraints(velocity); }
		const Vector3&			GetVelocity() const { return _velocity; }
		void					ClearVerticalVelocity() { _velocity.y = 0.f; }

		void					AddForce(const Vector3& force) { _accumulatedForce += ApplyPositionConstraints(force); }
		void					AddImpulse(const Vector3& impulse)
		{
			GM_ASSERT_RETURN(_mass > 0.f, "Rigidbody3D의 mass는 0보다 커야 합니다.");
			_velocity += ApplyPositionConstraints(impulse) / _mass;
		}

		void					ClearForces() { _accumulatedForce = Vector3{}; }

		void					SetMass(float mass)
		{
			GM_ASSERT_RETURN(mass > 0.f, "Rigidbody3D의 mass는 0보다 커야 합니다.");
			_mass = mass;
		}
		float					GetMass() const { return _mass; }

		void					SetCollisionResponseWeight(float weight)
		{
			GM_ASSERT_RETURN(weight > 0.f, "Collision Response Weight는 0보다 커야 합니다.");
			_collisionResponseWeight = weight;
		}
		float					GetCollisionResponseWeight() const { return _collisionResponseWeight; }

		void					SetGravityScale(float gravityScale) { _gravityScale = gravityScale; }
		float					GetGravityScale() const { return _gravityScale; }

		void					SetLinearDamping(float linearDamping)
		{
			GM_ASSERT_RETURN(linearDamping >= 0.f, "linearDamping은 0 이상이어야 합니다.");
			_linearDamping = linearDamping;
		}
		float					GetLinearDamping() const { return _linearDamping; }

		void					SetMaxSpeed(float maxSpeed)
		{
			GM_ASSERT_RETURN(maxSpeed >= 0.f, "maxSpeed는 0 이상이어야 합니다.");
			_maxSpeed = maxSpeed;
		}
		float					GetMaxSpeed() const { return _maxSpeed; }

		void					SetUseGravity(bool useGravity) { _useGravity = useGravity; }
		bool					IsUseGravity() const { return _useGravity; }

		void					SetKinematic(bool isKinematic) { _isKinematic = isKinematic; }
		bool					IsKinematic() const { return _isKinematic; }

		void					SetPositionConstraints(RigidbodyPositionConstraint constraints)
		{
			_positionConstraints = constraints;
			_velocity = ApplyPositionConstraints(_velocity);
			_accumulatedForce = ApplyPositionConstraints(_accumulatedForce);
		}

		RigidbodyPositionConstraint GetPositionConstraints() const { return _positionConstraints; }

	private:
		bool HasPositionConstraint(RigidbodyPositionConstraint constraint) const
		{
			return (static_cast<uint8>(_positionConstraints) & static_cast<uint8>(constraint)) != 0;
		}

		Vector3 ApplyPositionConstraints(Vector3 value) const
		{
			if (HasPositionConstraint(RigidbodyPositionConstraint::FreezeX))
				value.x = 0.f;
			if (HasPositionConstraint(RigidbodyPositionConstraint::FreezeY))
				value.y = 0.f;
			if (HasPositionConstraint(RigidbodyPositionConstraint::FreezeZ))
				value.z = 0.f;
			return value;
		}

	private:
		Vector3					_velocity{};
		Vector3					_accumulatedForce{};
		float					_mass = 1.f;
		float					_collisionResponseWeight = 1.f;
		float					_gravityScale = 1.f;
		float					_linearDamping = 0.f;
		float					_maxSpeed = 0.f;
		RigidbodyPositionConstraint _positionConstraints = RigidbodyPositionConstraint::None;

		bool					_useGravity = true;
		bool					_isKinematic = false;
	};
}
