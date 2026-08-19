#pragma once

#include <string>
#include "Widget.h"
#include "TextTypes.h"

namespace gm
{
	class TextBlock : public Widget
	{
	public:
		TextBlock();

		void				SetText(const std::wstring& text) { _text = text; }
		const std::wstring& GetText() const { return _text; }
		
		void				SetSize(float size) { _size = size; }
		float				GetSize() const { return _size; }

		void				SetColor(Color color) { _color = color; }
		Color				GetColor() const { return _color; }
		void				SetFont(const std::wstring& fontKey) { _fontKey = fontKey; }
		const std::wstring& GetFont() const { return _fontKey; }

		TextHorizontalAlignment		GetHorizonAlign() const { return _horizonAlign; }
		TextVerticalAlignment		GetVerticalAlign() const { return _verticalAlign; }
		void						SetHorizonAlign(TextHorizontalAlignment horizonAlign) { _horizonAlign = horizonAlign; }
		void						SetVerticalAlign(TextVerticalAlignment verticalAlign) { _verticalAlign = verticalAlign; }

protected:
		void				OnRender(const WidgetGeometry& geometry) override;

	private:
		std::wstring			_text{};
		std::wstring			_fontKey{};
		Color					_color = Colors::White;
		float					_size = 24.f;

		TextHorizontalAlignment _horizonAlign = TextHorizontalAlignment::Left;
		TextVerticalAlignment	_verticalAlign = TextVerticalAlignment::Top;
	};
}
