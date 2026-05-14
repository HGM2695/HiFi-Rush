#include "DebugRenderer.h"
#include <vector>

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

	void DebugRenderer::Render()
	{
#ifdef _DEBUG
		// TODO: DX11 기반 debug primitive 렌더링으로 교체합니다.
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
