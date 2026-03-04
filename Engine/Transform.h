#pragma once
#include "Component.h"

namespace gm
{
	class Transform : public Component
	{
	public:
		Transform();
		virtual ~Transform();

		virtual void	OnInitialize() override;
		virtual void	OnUpdate() override;
		virtual void	OnLateUpdate() override;
		virtual void	OnRender(HDC hDC) override;

		void			SetPos(int x, int y) { _X = x; _Y = y; }
		int				GetX() { return _X; }
		int				GetY() { return _Y; }

	private:
		int _X{};
		int _Y{};
	};
}