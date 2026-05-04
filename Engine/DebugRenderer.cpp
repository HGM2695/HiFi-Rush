#include "DebugRenderer.h"
#include "Camera.h"
#include <vector>
#include <windows.h>

namespace gm::debug
{
	namespace
	{
		struct DebugLine
		{
			Vector2 worldStart;
			Vector2 worldEnd;
			Color	color;
		};

		struct DebugRect
		{
			Vector2 worldCenter;
			Vector2 size;
			Color	color;
		};

		struct DebugCircle
		{
			Vector2 worldCenter;
			float	radius;
			Color	color;
		};

		struct DebugText
		{
			std::wstring	content;
			Vector2			viewPosition;
			Color			color;
		};

		std::vector<DebugLine> g_lines;
		std::vector<DebugRect> g_rects;
		std::vector<DebugCircle> g_circles;
		std::vector<DebugText> g_Texts;


		COLORREF ToColorRef(Color color)
		{
			return RGB(color.r, color.g, color.b);
		}
	}

	void DebugRenderer::RequestDrawLine(const Vector2& worldStart, const Vector2& worldEnd, Color color)
	{
#ifdef _DEBUG
		g_lines.emplace_back(worldStart, worldEnd, color);
#endif
	}

	void DebugRenderer::RequestDrawRect(const Vector2& worldCenter, const Vector2& size, Color color)
	{
#ifdef _DEBUG
		g_rects.emplace_back(worldCenter, size, color);
#endif
	}

	void DebugRenderer::RequestDrawCircle(const Vector2& worldCenter, float radius, Color color)
	{
#ifdef _DEBUG
		g_circles.emplace_back(worldCenter, radius, color);
#endif
	}

	void DebugRenderer::RequestDrawText(const std::wstring& content, const gm::Vector2& viewPosition, gm::Color color)
	{
#ifdef _DEBUG
		g_Texts.emplace_back(content, viewPosition, color);
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
			const Vector2 start = Camera::MainWorldToScreen(line.worldStart);
			const Vector2 end = Camera::MainWorldToScreen(line.worldEnd);
			HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(line.color));
			HGDIOBJ oldPen = SelectObject(hDC, pen);

			MoveToEx(hDC, static_cast<int>(start.x), static_cast<int>(start.y), nullptr);
			LineTo(hDC, static_cast<int>(end.x), static_cast<int>(end.y));

			SelectObject(hDC, oldPen);
			DeleteObject(pen);
		}

		for (const DebugRect& rect : g_rects)
		{
			const Vector2 screenCenter = Camera::MainWorldToScreen(rect.worldCenter);
			const Vector2 halfSize = rect.size * 0.5f;
			HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(rect.color));
			HGDIOBJ oldPen = SelectObject(hDC, pen);

			Rectangle(
				hDC,
				static_cast<int>(screenCenter.x - halfSize.x),
				static_cast<int>(screenCenter.y - halfSize.y),
				static_cast<int>(screenCenter.x + halfSize.x),
				static_cast<int>(screenCenter.y + halfSize.y)
			);

			SelectObject(hDC, oldPen);
			DeleteObject(pen);
		}

		for (const DebugCircle& circle : g_circles)
		{
			const Vector2 screenCenter = Camera::MainWorldToScreen(circle.worldCenter);
			HPEN pen = CreatePen(PS_SOLID, 1, ToColorRef(circle.color));
			HGDIOBJ oldPen = SelectObject(hDC, pen);

			Ellipse(
				hDC,
				static_cast<int>(screenCenter.x - circle.radius),
				static_cast<int>(screenCenter.y - circle.radius),
				static_cast<int>(screenCenter.x + circle.radius),
				static_cast<int>(screenCenter.y + circle.radius)
			);

			SelectObject(hDC, oldPen);
			DeleteObject(pen);
		}

		const int oldBkMode = SetBkMode(hDC, TRANSPARENT);
		for (const DebugText& text : g_Texts)
		{
			const COLORREF oldTextColor = SetTextColor(hDC, ToColorRef(text.color));

			TextOutW(
				hDC,
				static_cast<int>(text.viewPosition.x),
				static_cast<int>(text.viewPosition.y),
				text.content.c_str(),
				static_cast<int>(text.content.size())
			);

			SetTextColor(hDC, oldTextColor);
		}
		SetBkMode(hDC, oldBkMode);

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
		g_Texts.clear();
#endif
	}
}
