#pragma once

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class GameObject;

	class Component
	{
	public:
		Component();
		virtual ~Component();

		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC); 

		void				SetOwner(GameObject* owner) { _owner = owner; }
		GameObject&			GetOwner();
		const GameObject&	GetOwner() const;

	protected:
		virtual void	OnInitialize() {};
		virtual void	OnUpdate() {};
		virtual void	OnLateUpdate() {};
		virtual void	OnRender(HDC hDC) {};

	private:
		GameObject* _owner{};
	};
}

