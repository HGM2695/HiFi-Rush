#pragma once

#include "EngineCore.h"
#include "TickGroup.h"

namespace gm
{
	class GameObject;

	class Component
	{
	public:
		Component();
		virtual ~Component();

		void			Initialize();
		void			Tick(float deltaTime);
		void			Render(); 

		virtual TickGroup	GetTickGroup() const { return TickGroup::GameLogic; }

		void				SetEnabled(bool isEnabled) { _isEnabled = isEnabled; }
		bool				IsEnabled() const { return _isEnabled; }

		void				SetOwner(GameObject* owner) { _owner = owner; }
		GameObject&			GetOwner();
		const GameObject&	GetOwner() const;

	protected:
		virtual void	OnInitialize() {};
		virtual void	OnTick(float deltaTime) {};
		virtual void	OnRender() {};

	private:
		GameObject*		_owner{};
		bool			_isEnabled = true;
	};
}

