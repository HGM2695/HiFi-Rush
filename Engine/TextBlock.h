#pragma once

#include <string>
#include "Color.h"
#include "Widget.h"

namespace gm
{
	class TextBlock : public Widget
	{
	public:
		void				SetText(const std::wstring& text) { _text = text; }
		const std::wstring& GetText() const { return _text; }

		void				SetColor(Color color) { _color = color; }
		Color				GetColor() const { return _color; }

	protected:
		void OnRender(HDC hDC, const Vector2& absolutePosition) override;

	private:
		std::wstring	_text{};
		Color			_color = Color::White();
	};
}
