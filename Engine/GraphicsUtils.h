#pragma once

#include "BoundingTypes.h"
#include "Types.h"

namespace gm
{
	inline constexpr uint32 Align16(uint32 size)
	{
		return ((size + 15) / 16) * 16;
	}

	inline BoundingFrustum CreateWorldFrustum(const Matrix& view, const Matrix& projection)
	{
		BoundingFrustum viewFrustum{};
		BoundingFrustum::CreateFromMatrix(viewFrustum, projection);

		BoundingFrustum worldFrustum{};
		viewFrustum.Transform(worldFrustum, view.Invert());
		return worldFrustum;
	}

	inline bool IsBoundingVolumeVisible(const BoundingFrustum& frustum, const BoundingVolume& bounds)
	{
		if (bounds.isValid == false)
			return true;

		return frustum.Intersects(bounds.box);
	}
}
