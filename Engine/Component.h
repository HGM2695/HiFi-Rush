#pragma once

#include "Entity.h"
#include <cassert>

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class GameObject;

	class Component : public Entity
	{
	public:
		Component();
		virtual ~Component();

		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC); 

		void			SetOwner(GameObject* owner) { _Owner = owner; }
		GameObject& GetOwner()
		{
			assert(_Owner && "컴포넌트는 항상 소유자가 있어야 합니다.");
			return *_Owner;
		}

		const GameObject& GetOwner() const
		{
			assert(_Owner && "컴포넌트는 항상 소유자가 있어야 합니다.");
			return *_Owner;
		}

	protected:
		virtual void	OnInitialize() {};
		virtual void	OnUpdate() {};
		virtual void	OnLateUpdate() {};
		virtual void	OnRender(HDC hDC) {};

	private:
		GameObject* _Owner{};
	};
}

