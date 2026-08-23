#pragma once

#include "MaterialTypes.h"

namespace gm
{
	// G-Buffer 내부의 각 Target의 의미와 MRT 순서
	enum class GBufferTarget
	{
		BaseColor,
		WorldNormal,
		MaterialData,
		Emissive,

		Count
	};

	inline constexpr uint32 ToGBufferTargetIndex(GBufferTarget target) { return static_cast<uint32>(target); }
	inline constexpr uint32 GBufferTargetCount = ToGBufferTargetIndex(GBufferTarget::Count);

	namespace GBufferMaterialDataLayout
	{
		inline constexpr uint32 AmbientOcclusionChannel = 0;

		inline constexpr uint32 MaterialFlagsChannel = 3;
		inline constexpr uint32 ShadingModelMask = (1 << 0) | (1 << 1);
		inline constexpr uint32 OutlineEnabledBit = 1 << 2;
		static_assert(static_cast<uint32>(ShadingModel::Count) <= ShadingModelMask + 1, "ShadingModel 개수가 G-Buffer Material Flag의 비트 범위를 초과했습니다.");

		inline constexpr uint32 PackFlags(ShadingModel shadingModel, bool outlineEnabled)
		{
			uint32 flags = static_cast<uint32>(shadingModel) & ShadingModelMask;
			if (outlineEnabled)
				flags |= OutlineEnabledBit;
			return flags;
		}
	}
}
