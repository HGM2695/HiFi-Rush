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
	}

	void GameObject::LateUpdate()
	{
	}

	void GameObject::Render(HDC hDC)
	{
		Rectangle(hDC, _x, _y, _x + 100, _y + 100);
	}
}

