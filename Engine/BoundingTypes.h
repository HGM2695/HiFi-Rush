#pragma once

#include "MathTypes.h"

namespace gm
{
	struct BoundingVolume
	{
		BoundingBox	box{};
		bool		isValid = false;
	};

	inline BoundingVolume TransformBoundingVolume(const BoundingVolume& localBounds, const Matrix& transform)
	{
		BoundingVolume worldBounds{};
		if (localBounds.isValid == false)
			return worldBounds;

		localBounds.box.Transform(worldBounds.box, transform);
		worldBounds.isValid = true;
		return worldBounds;
	}
}
