#include "Widget.h"

namespace gm
{
	Vector2 Widget::ResolveSize(const WidgetGeometry& parentGeometry) const
	{
		switch (_sizeRule)
		{
		case WidgetSizeRule::Fixed:
			return _size;
		case WidgetSizeRule::FillParent:
			return parentGeometry.size;
		default:
			GM_ASSERT_RETURN_VAL(false, _size, "지원하지 않는 WidgetSizeRule입니다.");
		}
	}

	void Widget::Initialize()
	{
		if (_isInitialized)
			return;

		OnInitialize();
		_isInitialized = true;

		for (const auto& child : _childList)
			child->Initialize();
	}

	void Widget::Tick(float deltaTime)
	{
		if (_isVisible == false)
			return;

		OnTick(deltaTime);

		for (const auto& child : _childList)
			child->Tick(deltaTime);
	}

	void Widget::Render(const WidgetGeometry& parentGeometry)
	{
		if (_isVisible == false)
			return;

		const WidgetGeometry geometry
		{
			parentGeometry.center + _position,
			ResolveSize(parentGeometry)
		};

		OnRender(geometry);

		for (const auto& child : _childList)
			child->Render(geometry);
	}
}
