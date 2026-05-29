#pragma once

#include "GraphicsTypes.h"
#include "Types.h"
#include <array>
#include <string>

namespace gm
{
	struct MeshSection
	{
		std::wstring	name;
		uint32			indexStart = 0;
		uint32			indexCount = 0;
		uint32			textureSlotIndex = 0;
	};

	struct MeshTextureSlot
	{
		std::array<std::wstring, TextureSlotCount> textureKeys{};
	};
}
