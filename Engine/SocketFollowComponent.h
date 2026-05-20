#pragma once

#include "Component.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class GameObject;
	class SocketComponent;
	class Transform;

	class SocketFollowComponent : public Component
	{
	public:
		virtual TickGroup GetTickGroup() const override { return TickGroup::Attachment; }

		void						SetTarget(const GameObject& target, const std::wstring& socketName);
		void						ClearTarget();

		const WeakGameObjectPtr&	GetTargetObject() const { return _target; }
		const std::wstring&			GetFollowSocketName() const { return _followSocketName; }

	protected:
		virtual void				OnInitialize() override;
		virtual void				OnTick(float deltaTime) override;

	private:
		Transform*				_ownerTransform = nullptr;
		WeakGameObjectPtr		_target;
		const SocketComponent*	_targetSocketComponent = nullptr;
		std::wstring			_followSocketName;
	};
}
