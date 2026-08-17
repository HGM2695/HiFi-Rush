#pragma once

#include "Component.h"
#include "Event.h"

namespace gm
{
	class Rigidbody3DComponent;
	class TransformComponent;

	struct NavigationGroundContactEvent final : EventType
	{
		Vector3	position{};
		int32	cellIndex = -1;
	};

	struct NavigationGroundLostEvent final : EventType
	{
		Vector3	position{};
		float	groundHeight = 0.f;
		int32	cellIndex = -1;
	};

	class NavMeshControllerComponent : public Component
	{
	private:
		enum class GroundState
		{
			Uninitialized,
			Airborne,
			Grounded,
		};

	public:
		virtual TickGroup	GetTickGroup() const override { return TickGroup::PostPhysics; }

		void				SetUseGroundCollision(bool useGroundCollision)
		{
			_useGroundCollision = useGroundCollision;
			_groundState = GroundState::Uninitialized;
			_lastValidGroundCellIndex = -1;
			_hasLastValidGroundPosition = false;
		}
		bool				IsUseGroundCollision() const { return _useGroundCollision; }
		bool				IsGrounded() const { return _groundState == GroundState::Grounded; }
		void				SetMaxGroundSnapDownDistance(float distance);
		float				GetMaxGroundSnapDownDistance() const { return _maxGroundSnapDownDistance; }

		int32				GetCurrentCellIndex() const { return _currentCellIndex; }
		void				SetCurrentCellIndex(int32 cellIndex) { _currentCellIndex = cellIndex; }
		bool				RefreshCellIndex();
		bool				Move(const Vector3& desiredDelta);

		EventPublisher<NavMeshControllerComponent, NavigationGroundContactEvent>	OnGroundContact;
		EventPublisher<NavMeshControllerComponent, NavigationGroundLostEvent>		OnGroundLost;

	protected:
		virtual void		OnInitialize() override;
		virtual void		OnTick(float deltaTime) override;

	private:
		void				CheckGroundCollision();
		void				UpdateLastValidGroundPosition(const Vector3& position, int32 cellIndex);
		bool				RestoreLastValidGroundPosition(Vector3& position);

	private:
		TransformComponent*		_transform = nullptr;
		Rigidbody3DComponent*	_rigidbody = nullptr;
		int32					_currentCellIndex = -1;
		float					_maxGroundSnapDownDistance = 0.1f;
		Vector3					_lastValidGroundPosition{};
		int32					_lastValidGroundCellIndex = -1;
		GroundState				_groundState = GroundState::Uninitialized;
		bool					_useGroundCollision = false;
		bool					_hasLastValidGroundPosition = false;
	};
}
