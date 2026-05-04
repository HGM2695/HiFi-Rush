#pragma once

#include "EngineCore.h"
#include <string>

namespace gm::debug
{
	class DebugRenderer
	{
	public:
		static void RequestDrawLine(const gm::Vector2& worldStart, const gm::Vector2& worldEnd, gm::Color color = gm::Colors::Green);
		static void RequestDrawRect(const gm::Vector2& worldCenter, const gm::Vector2& size, gm::Color color = gm::Colors::Green);
		static void RequestDrawCircle(const gm::Vector2& worldCenter, float radius, gm::Color color = gm::Colors::Green);
		static void RequestDrawText(const std::wstring& content, const gm::Vector2& viewPosition, gm::Color color = gm::Colors::Black);
		static void Render(HDC hDC);
		static void Clear();
	};
}
