#include "Border.h"
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

	void Border::OnRender(HDC hDC, const math::Vector2& absolutePosition)
	{
		HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(_outlineColor));
		HBRUSH brush = CreateSolidBrush(ToColorRef(_backgroundColor));
		HGDIOBJ oldPen = SelectObject(hDC, pen);
		HGDIOBJ oldBrush = SelectObject(hDC, brush);

		Rectangle(
			hDC,
			static_cast<int>(absolutePosition._x),
			static_cast<int>(absolutePosition._y),
			static_cast<int>(absolutePosition._x + GetSize()._x),
			static_cast<int>(absolutePosition._y + GetSize()._y)
		);

		SelectObject(hDC, oldBrush);
		SelectObject(hDC, oldPen);
		DeleteObject(brush);
		DeleteObject(pen);
	}
}
