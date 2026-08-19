#include "MovementBaseComponent.h"

#include "CharacterMovementComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <utility>

namespace gm
{
	namespace
	{
		constexpr float MinimumTopContactDot = 0.7f;
	}

	MovementBaseComponent::MovementBaseComponent(std::wstring colliderId, CollisionMask passengerMask)
		: _colliderId(std::move(colliderId)), _passengerMask(passengerMask)
	{}

	Vector3 MovementBaseComponent::CalculateMovementDelta(const Vector3& worldPosition) const
	{
		if (_transform == nullptr)
			return Vector3{};

		//이전 발판의 변환을 제거하고 현재 발판의 변환을 다시 적용 (즉 변환의 델타를 적용)
		const Vector3 baseLocalPosition = Vector3::Transform(worldPosition, _previousWorld.Invert());
		const Vector3 movedWorldPosition = Vector3::Transform(baseLocalPosition, _transform->GetWorldMatrix());
		return movedWorldPosition - worldPosition;
	}

	void MovementBaseComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_colliderId.empty() == false, "MovementBaseComponent의 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN(_passengerMask != 0, "MovementBaseComponent의 Passenger Mask는 비어 있을 수 없습니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "MovementBaseComponent는 TransformComponent가 필요합니다.");

		for (Collider3DComponent* collider : GetOwner().GetColliders3D())
		{
			if (collider != nullptr && collider->GetColliderId() == _colliderId)
			{
				_collider = collider;
				break;
			}
		}

		GM_ASSERT_RETURN(_collider, "MovementBaseComponent가 참조하는 Collider를 찾지 못했습니다. colliderId=%ls", _colliderId.c_str());
		GM_ASSERT_RETURN(_collider->IsTrigger() == false, "MovementBaseComponent는 Trigger Collider를 사용할 수 없습니다. colliderId=%ls", _colliderId.c_str());

		_previousWorld = _transform->GetWorldMatrix();
		_collider->OnCollisionEnter.Subscribe(_collisionEnterConnection,
			[this](const Collision3DEvent& event)
			{
				HandleContact(event);
			});

		_collider->OnCollisionStay.Subscribe(_collisionStayConnection,
			[this](const Collision3DEvent& event)
			{
				HandleContact(event);
			});

		_collider->OnCollisionExit.Subscribe(_collisionExitConnection,
			[this](const Collision3DEvent& event)
			{
				HandleExit(event);
			});
	}

	void MovementBaseComponent::OnTick(float)
	{
		if (_transform)
			_previousWorld = _transform->GetWorldMatrix();
	}

	void MovementBaseComponent::HandleContact(const Collision3DEvent& event)
	{
		if (event.otherCollider == nullptr || IsPassenger(*event.otherCollider) == false)
			return;

		CharacterMovementComponent* movement = event.otherCollider->GetOwner().GetComponent<CharacterMovementComponent>();
		if (movement == nullptr)
			return;

		if (IsTopContact(event.contact))
			movement->SetMovementBase(*this);
		else
			movement->ClearMovementBase(*this);
	}

	void MovementBaseComponent::HandleExit(const Collision3DEvent& event)
	{
		if (event.otherCollider == nullptr || IsPassenger(*event.otherCollider) == false)
			return;

		CharacterMovementComponent* movement = event.otherCollider->GetOwner().GetComponent<CharacterMovementComponent>();
		if (movement)
			movement->ClearMovementBase(*this);
	}

	bool MovementBaseComponent::IsPassenger(const Collider3DComponent& collider) const
	{
		return (_passengerMask & collider.GetCollisionLayer()) != 0;
	}

	bool MovementBaseComponent::IsTopContact(const CollisionContact& contact) const
	{
		if (_transform == nullptr)
			return false;

		const Vector3 baseUp = Math::GetUpVector(_transform->GetRotation());
		const Vector3 directionToPassenger = -contact.normal;
		return baseUp.Dot(directionToPassenger) >= MinimumTopContactDot;
	}
}
