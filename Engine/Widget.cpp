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
