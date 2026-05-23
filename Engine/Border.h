#pragma once

#include "Widget.h"

namespace gm
{
	class Border : public Widget
	{
	public:
		Border();

		void	SetBackgroundColor(Color color) { _backgroundColor = color; }
		void	SetOutlineColor(Color color) { _outlineColor = color; }
		Color	GetBackgroundColor() const { return _backgroundColor; }
		Color	GetOutlineColor() const { return _outlineColor; }

		void	SetBorderThickness(float thickness) { _borderThickness = thickness; }
		float	GetBorderThickness() const { return _borderThickness; }


protected:
		void OnRender(const WidgetGeometry& geometry) override;
		void SubmitBackGround(const WidgetGeometry& geometry);
		void SubmitOutline(const WidgetGeometry& geometry);

	private:
		Color _backgroundColor = Colors::Black;
		Color _outlineColor = Colors::White;
		float _borderThickness = 1.f;
	};
}
