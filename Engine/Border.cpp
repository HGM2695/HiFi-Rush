#include "Border.h"
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
