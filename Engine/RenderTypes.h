#pragma once

#include "MathTypes.h"
#include "SpriteFrame.h"

namespace gm
{
	class Material;

	struct SpriteRenderItem
	{
		Matrix			world = Matrix::CreateScale(1.f);
		const Material* material = nullptr;
		bool			useSourceRect = false;
		SpriteFrame		sourceFrame{};
	};
}
