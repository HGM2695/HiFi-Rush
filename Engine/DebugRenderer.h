#pragma once

#include "EngineCore.h"
#include "Color.h"
#include "Vector2.h"
#include <string>

namespace gm::debug
{
	class DebugRenderer
	{
	public:
		static void RequestDrawLine(const gm::math::Vector2& worldStart, const gm::math::Vector2& worldEnd, gm::Color color = gm::Color::Green());
		static void RequestDrawRect(const gm::math::Vector2& worldCenter, const gm::math::Vector2& size, gm::Color color = gm::Color::Green());
		static void RequestDrawCircle(const gm::math::Vector2& worldCenter, float radius, gm::Color color = gm::Color::Green());
		static void RequestDrawText(const std::wstring& content, const gm::math::Vector2& viewPosition, gm::Color color = gm::Color::Black());
		static void Render(HDC hDC);
		static void Clear();
	};
}
