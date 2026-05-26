#pragma once

#include "Widget.h"
#include <memory>

namespace gm
{
	class Material;

	class Border : public Widget
	{
	public:
		Border();
		~Border();

		void	SetBackgroundColor(Color color);
		void	SetOutlineColor(Color color);
		Color	GetBackgroundColor() const { return _backgroundColor; }
		Color	GetOutlineColor() const { return _outlineColor; }

		void	SetBorderThickness(float thickness) { _borderThickness = thickness; }
		float	GetBorderThickness() const { return _borderThickness; }


protected:
		void		OnRender(const WidgetGeometry& geometry) override;
		void		SubmitBackGround(const WidgetGeometry& geometry);
		void		SubmitOutline(const WidgetGeometry& geometry);
		void		CreateBackgroundMaterial();
		void		CreateOutlineMaterial();
		void		UpdateBackgroundMaterial();
		void		UpdateOutlineMaterial();

	private:
		Color						_backgroundColor = Colors::Black;
		Color						_outlineColor = Colors::White;
		float						_borderThickness = 1.f;
		std::unique_ptr<Material>	_backgroundMaterial;
		std::unique_ptr<Material>	_outlineMaterial;
	};
}
