#pragma once

#include "Component.h"

#include <memory>

namespace gm
{
	class NavMeshMoveController;
	class TransformComponent;

	class MovementComponent : public Component
	{
	public:
		MovementComponent();
		virtual ~MovementComponent();
		virtual TickGroup	GetTickGroup() const override { return TickGroup::Movement; }

		void				EnableNavigationMovement(bool enabled);
		bool				IsNavigationMovementEnabled() const;
		bool				RefreshNavigationCellIndex();

	protected:
		virtual void		OnInitialize() override;

		void				Move(const Vector3& desiredDelta);
		void				Translate(const Vector3& desiredDelta);
		void				SetPosition(const Vector3& position);
		Vector3				GetPosition() const;
		TransformComponent* GetOwnerTransform() const { return _ownerTransform; }

	private:
		TransformComponent*						_ownerTransform = nullptr;
		std::unique_ptr<NavMeshMoveController>	_navMeshMoveController;
	};
}
