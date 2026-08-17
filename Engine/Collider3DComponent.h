#pragma once

#include "CollisionTypes.h"
#include "Component.h"
#include "Event.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class PhysicsSystem3D;

	struct Collision3DEvent : EventType
	{
		Collider3DComponent*	selfCollider = nullptr;
		Collider3DComponent*	otherCollider = nullptr;
		CollisionType			type = CollisionType::Contact;
		CollisionContact		contact{};
	};

	class Collider3DComponent : public Component
	{
	friend class PhysicsSystem3D;

	public:
		virtual ~Collider3DComponent() = default;

		ColliderShape3DType		GetShapeType() const { return _shapeType; }
		void					SetColliderId(const std::wstring& colliderId);
		const std::wstring&		GetColliderId() const { return _colliderId; }

		void					SetLocalCenter(const Vector3& center) { _localCenter = center; }
		const Vector3&			GetLocalCenter() const { return _localCenter; }
		void					SetLocalRotation(const Quaternion& rotation) { _localRotation = rotation; }
		const Quaternion&		GetLocalRotation() const { return _localRotation; }

		void					SetCollisionFilter(const CollisionFilter& filter);
		const CollisionFilter&	GetCollisionFilter() const { return _collisionFilter; }
		void					SetCollisionLayer(CollisionLayer layer);
		CollisionLayer			GetCollisionLayer() const { return _collisionFilter.layer; }
		void					SetCollisionMask(CollisionMask mask) { _collisionFilter.mask = mask; }
		void					AddCollisionLayerToMask(CollisionLayer layer);
		void					RemoveCollisionLayerFromMask(CollisionLayer layer);
		CollisionMask			GetCollisionMask() const { return _collisionFilter.mask; }

		void					SetTrigger(bool isTrigger) { _isTrigger = isTrigger; }
		bool					IsTrigger() const { return _isTrigger; }
		void					SetCollisionResponseMode(CollisionResponseMode responseMode) { _collisionResponseMode = responseMode; }
		CollisionResponseMode	GetCollisionResponseMode() const { return _collisionResponseMode; }

		EventPublisher<Collider3DComponent, Collision3DEvent>	OnCollisionEnter;
		EventPublisher<Collider3DComponent, Collision3DEvent>	OnCollisionStay;
		EventPublisher<Collider3DComponent, Collision3DEvent>	OnCollisionExit;

	protected:
		explicit Collider3DComponent(ColliderShape3DType shapeType) : _shapeType(shapeType) {}

	private:
		virtual void UpdateWorldShape() = 0;
		void DispatchCollisionEvent(CollisionState state, CollisionType type, const CollisionContact& contact, Collider3DComponent& otherCollider);

		ColliderShape3DType		_shapeType;
		std::wstring			_colliderId{};
		Vector3					_localCenter{};
		Quaternion				_localRotation{ 0.f, 0.f, 0.f, 1.f };
		CollisionFilter			_collisionFilter{};
		CollisionResponseMode	_collisionResponseMode = CollisionResponseMode::Full;
		bool					_isTrigger = false;
	};
}
