#include "DebugRenderer.h"
#include "Camera.h"
#include "GMAssert.h"
#include <vector>
#include <windows.h>

namespace gm::debug
{
	namespace
	{
		struct DebugLine
		{
			math::Vector2 worldStart;
			math::Vector2 worldEnd;
			Color color;
		};

		struct DebugRect
		{
			math::Vector2 worldCenter;
			math::Vector2 size;
			Color color;
		};

		struct DebugCircle
		{
			math::Vector2 worldCenter;
			float radius;
			Color color;
		};

		std::vector<DebugLine> g_lines;
		std::vector<DebugRect> g_rects;
		std::vector<DebugCircle> g_circles;

		COLORREF ToColorRef(Color color)
		{
			return RGB(color.r, color.g, color.b);
		}
	}

	void DebugRenderer::RequestDrawLine(const math::Vector2& worldStart, const math::Vector2& worldEnd, Color color)
	{
#ifdef _DEBUG
		g_lines.push_back({ worldStart, worldEnd, color });
#endif
	}

	void DebugRenderer::RequestDrawRect(const math::Vector2& worldCenter, const math::Vector2& size, Color color)
	{
#ifdef _DEBUG
		g_rects.push_back({ worldCenter, size, color });
#endif
	}

	void DebugRenderer::RequestDrawCircle(const math::Vector2& worldCenter, float radius, Color color)
	{
#ifdef _DEBUG
		g_circles.push_back({ worldCenter, radius, color });
#endif
	}

	void DebugRenderer::Render(HDC hDC)
	{
#ifdef _DEBUG
		GM_ASSERT_RETURN(hDC, "DC가 nullptr입니다.");

		// 속을 채워넣지 않기 위해, HOLLOW_BRUSH
		HGDIOBJ oldBrush = SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));

		for (const DebugLine& line : g_lines)
		{
			const math::Vector2 start = Camera::MainWorldToScreen(line.worldStart);
			const math::Vector2 end = Camera::MainWorldToScreen(line.worldEnd);
			HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(line.color));
			HGDIOBJ oldPen = SelectObject(hDC, pen);

			MoveToEx(hDC, static_cast<int>(start._x), static_cast<int>(start._y), nullptr);
			LineTo(hDC, static_cast<int>(end._x), static_cast<int>(end._y));

			SelectObject(hDC, oldPen);
			DeleteObject(pen);
		}

		for (const DebugRect& rect : g_rects)
		{
			const math::Vector2 screenCenter = Camera::MainWorldToScreen(rect.worldCenter);
			const math::Vector2 halfSize = rect.size * 0.5f;
			HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(rect.color));
			HGDIOBJ oldPen = SelectObject(hDC, pen);

			Rectangle(
				hDC,
				static_cast<int>(screenCenter._x - halfSize._x),
				static_cast<int>(screenCenter._y - halfSize._y),
				static_cast<int>(screenCenter._x + halfSize._x),
				static_cast<int>(screenCenter._y + halfSize._y)
			);

			SelectObject(hDC, oldPen);
			DeleteObject(pen);
		}

		for (const DebugCircle& circle : g_circles)
		{
			const math::Vector2 screenCenter = Camera::MainWorldToScreen(circle.worldCenter);
			HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(circle.color));
			HGDIOBJ oldPen = SelectObject(hDC, pen);

			Ellipse(
				hDC,
				static_cast<int>(screenCenter._x - circle.radius),
				static_cast<int>(screenCenter._y - circle.radius),
				static_cast<int>(screenCenter._x + circle.radius),
				static_cast<int>(screenCenter._y + circle.radius)
			);

			SelectObject(hDC, oldPen);
			DeleteObject(pen);
		}

		SelectObject(hDC, oldBrush);
		Clear();
#endif
	}

	void DebugRenderer::Clear()
	{
#ifdef _DEBUG
		g_lines.clear();
		g_rects.clear();
		g_circles.clear();
#endif
	}
}
