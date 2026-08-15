#pragma once

#include "Component.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class GameObject;
	class SocketComponent;
	class TransformComponent;

	class SocketFollowComponent : public Component
	{
	public:
		virtual TickGroup GetTickGroup() const override { return TickGroup::Attachment; }

		void						SetTarget(const GameObject& target, const std::wstring& socketName);
		void						ClearTarget();
		void						SetDestroyWithTarget(bool destroyWithTarget) { _destroyWithTarget = destroyWithTarget; }

		const WeakGameObjectPtr&	GetTargetObject() const { return _target; }
		const std::wstring&			GetFollowSocketName() const { return _followSocketName; }

	protected:
		virtual void				OnInitialize() override;
		virtual void				OnTick(float deltaTime) override;

	private:
		TransformComponent*				_ownerTransform = nullptr;
		WeakGameObjectPtr		_target;
		const SocketComponent*	_targetSocketComponent = nullptr;
		std::wstring			_followSocketName;
		bool					_destroyWithTarget = false;
	};
}
