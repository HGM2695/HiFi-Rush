#pragma once

#include "Types.h"

namespace gm
{
	struct Rect
	{
		float left = 0.f;
		float top = 0.f;
		float width = 0.f;
		float height = 0.f;

		float Right() const { return left + width; }
		float Bottom() const { return top + height; }

		static Rect FromCenterSize(const Vector2 & center, const Vector2 & size)
		{
			return Rect
			{
				center.x - size.x * 0.5f,
				center.y - size.y * 0.5f,
				size.x,
				size.y
			};
		}
	};

	struct IntRect
	{
		int32 left = 0;
		int32 top = 0;
		int32 width = 0;
		int32 height = 0;

		int32 Right() const { return left + width; }
		int32 Bottom() const { return top + height; }
	};
}
