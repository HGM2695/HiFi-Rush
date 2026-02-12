#include "GameObject.h"
#include "Windows.h"

namespace gm
{
	GameObject::GameObject()
	{
	}

	GameObject::~GameObject()
	{
	}

	void GameObject::Update()
	{
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			_x -= 0.01f;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			_x += 0.01f;
		if (GetAsyncKeyState(VK_UP) & 0x8000)
			_y -= 0.01f;
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			_y += 0.01f;
	}

	void GameObject::LateUpdate()
	{
	}

	void GameObject::Render(HDC hdc)
	{
		Rectangle(hdc, _x, _y, _x + 100, _y + 100);
	}
}

