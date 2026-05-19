#include "Widget.h"

namespace gm
{
	void Widget::Tick(float deltaTime)
	{
		if (_isVisible == false)
			return;

		OnTick(deltaTime);

		for (const auto& child : _childList)
			child->Tick(deltaTime);
	}

	void Widget::Render(const Vector2& parentPosition)
	{
		if (_isVisible == false)
			return;

		const Vector2 absolutePosition = parentPosition + _position;
		OnRender(absolutePosition);

		for (const auto& child : _childList)
			child->Render(absolutePosition);
	}
}
