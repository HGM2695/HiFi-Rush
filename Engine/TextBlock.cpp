#include "TextBlock.h"
#include <algorithm>
#include <windows.h>

namespace gm
{
	namespace
	{
		BYTE ToByte(float color)
		{
			return static_cast<BYTE>(std::clamp(color, 0.f, 1.f) * 255.f + 0.5f);
		}

		COLORREF ToColorRef(Color color)
		{
			return RGB(ToByte(color.x), ToByte(color.y), ToByte(color.z));
		}
	}

	void TextBlock::OnRender(HDC hDC, const Vector2& absolutePosition)
	{
		if (_text.empty())
			return;

		const int oldBkMode = SetBkMode(hDC, TRANSPARENT);
		const COLORREF oldTextColor = SetTextColor(hDC, ToColorRef(_color));

		TextOutW(
			hDC,
			static_cast<int>(absolutePosition.x),
			static_cast<int>(absolutePosition.y),
			_text.c_str(),
			static_cast<int>(_text.size())
		);

		SetTextColor(hDC, oldTextColor);
		SetBkMode(hDC, oldBkMode);
	}
}
