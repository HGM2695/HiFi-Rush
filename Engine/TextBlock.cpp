#include "TextBlock.h"
#include <windows.h>

namespace gm
{
	namespace
	{
		COLORREF ToColorRef(Color color)
		{
			return RGB(color.r, color.g, color.b);
		}
	}

	void TextBlock::OnRender(HDC hDC, const math::Vector2& absolutePosition)
	{
		if (_text.empty())
			return;

		const int oldBkMode = SetBkMode(hDC, TRANSPARENT);
		const COLORREF oldTextColor = SetTextColor(hDC, ToColorRef(_color));

		TextOutW(
			hDC,
			static_cast<int>(absolutePosition._x),
			static_cast<int>(absolutePosition._y),
			_text.c_str(),
			static_cast<int>(_text.size())
		);

		SetTextColor(hDC, oldTextColor);
		SetBkMode(hDC, oldBkMode);
	}
}
