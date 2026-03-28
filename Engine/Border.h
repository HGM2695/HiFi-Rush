#pragma once

#include "Color.h"
#include "Widget.h"

namespace gm
{
	class Border : public Widget
	{
	public:
		void SetBackgroundColor(Color color) { _backgroundColor = color; }
		void SetOutlineColor(Color color) { _outlineColor = color; }

	protected:
		void OnRender(HDC hDC, const math::Vector2& absolutePosition) override;

	private:
		Color _backgroundColor = Color::Black();
		Color _outlineColor = Color::White();
	};
}
