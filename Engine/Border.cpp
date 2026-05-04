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

	void Border::OnRender(HDC hDC, const Vector2& absolutePosition)
	{
		HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(_outlineColor));
		HBRUSH brush = CreateSolidBrush(ToColorRef(_backgroundColor));
		HGDIOBJ oldPen = SelectObject(hDC, pen);
		HGDIOBJ oldBrush = SelectObject(hDC, brush);

		Rectangle(
			hDC,
			static_cast<int>(absolutePosition.x),
			static_cast<int>(absolutePosition.y),
			static_cast<int>(absolutePosition.x + GetSize().x),
			static_cast<int>(absolutePosition.y + GetSize().y)
		);

		SelectObject(hDC, oldBrush);
		SelectObject(hDC, oldPen);
		DeleteObject(brush);
		DeleteObject(pen);
	}
}
