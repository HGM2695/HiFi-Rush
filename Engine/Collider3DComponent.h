#pragma once

#include "CollisionTypes.h"
#include "Component.h"

namespace gm
{
	class Collider3DComponent : public Component
	{
	public:
		virtual ~Collider3DComponent() = default;

		ColliderShape3DType		GetShapeType() const { return _shapeType; }

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

	protected:
		explicit Collider3DComponent(ColliderShape3DType shapeType) : _shapeType(shapeType) {}

	private:
		ColliderShape3DType	_shapeType;
		Vector3			_localCenter{};
		Quaternion		_localRotation{ 0.f, 0.f, 0.f, 1.f };
		CollisionFilter	_collisionFilter{};
		bool			_isTrigger = false;
	};
}
