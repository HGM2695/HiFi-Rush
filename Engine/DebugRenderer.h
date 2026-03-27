#pragma once

#include "Color.h"
#include "Vector2.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm::debug
{
	class DebugRenderer
	{
	public:
		static void RequestDrawLine(const gm::math::Vector2& worldStart, const gm::math::Vector2& worldEnd, gm::Color color = gm::Color::Green());
		static void RequestDrawRect(const gm::math::Vector2& worldCenter, const gm::math::Vector2& size, gm::Color color = gm::Color::Green());
		static void RequestDrawCircle(const gm::math::Vector2& worldCenter, float radius, gm::Color color = gm::Color::Green());
		static void Render(HDC hDC);
		static void Clear();
	};
}
