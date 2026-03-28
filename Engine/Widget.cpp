#include "Widget.h"

namespace gm
{
	void Widget::Update()
	{
		if (_isVisible == false)
			return;

		OnUpdate();

		for (const auto& child : _childList)
			child->Update();
	}

	void Widget::Render(HDC hDC, const math::Vector2& parentPosition)
	{
		if (_isVisible == false)
			return;

		const math::Vector2 absolutePosition = parentPosition + _position;
		OnRender(hDC, absolutePosition);

		for (const auto& child : _childList)
			child->Render(hDC, absolutePosition);
	}
}
