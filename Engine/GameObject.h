#pragma once

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		void	Update();
		void	LateUpdate();
		void	Render(HDC hdc);

		void SetPosition(float x, float y)
		{
			_x = x;
			_y = y;
		}
		float GetPositionX() { return _x; }
		float GetPositionY() { return _y; }
		
	private:
		float _x;
		float _y;
	};
}


