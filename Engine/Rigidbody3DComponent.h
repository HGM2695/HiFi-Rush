#pragma once

#include "Component.h"

namespace gm
{
	class Rigidbody3DComponent : public Component
	{
	friend class PhysicsSystem3D;

	public:
		void					SetVelocity(const Vector3& velocity) { _velocity = velocity; }
		const Vector3&			GetVelocity() const { return _velocity; }

		void					AddForce(const Vector3& force) { _accumulatedForce += force; }
		void					AddImpulse(const Vector3& impulse)
		{
			GM_ASSERT_RETURN(_mass > 0.f, "Rigidbody3D의 mass는 0보다 커야 합니다.");
			_velocity += impulse / _mass;
		}

		void					ClearForces() { _accumulatedForce = Vector3{}; }

		void					SetMass(float mass)
		{
			GM_ASSERT_RETURN(mass > 0.f, "Rigidbody3D의 mass는 0보다 커야 합니다.");
			_mass = mass;
		}
		float					GetMass() const { return _mass; }

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

	private:
		Vector3					_velocity{};
		Vector3					_accumulatedForce{};
		float					_mass = 1.f;
		float					_gravityScale = 1.f;
		float					_linearDamping = 0.f;
		float					_maxSpeed = 0.f;

		bool					_useGravity = true;
		bool					_isKinematic = false;
	};
}
