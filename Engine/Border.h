#pragma once

#include "Widget.h"

namespace gm
{
	class Border : public Widget
	{
	public:
		void SetBackgroundColor(Color color) { _backgroundColor = color; }
		void SetOutlineColor(Color color) { _outlineColor = color; }

	protected:
		void OnRender(const Vector2& absolutePosition) override;

	private:
		Color _backgroundColor = Colors::Black;
		Color _outlineColor = Colors::White;
	};
}
