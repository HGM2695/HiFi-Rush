#pragma once

#include "Types.h"

#include <array>
#include <string>

namespace gm
{
	enum class SjangoAnimationId : uint32
	{
		Idle,
		Damage,

		Count,
	};

	inline constexpr uint32 ToIndex(SjangoAnimationId animationId)
	{
		return static_cast<uint32>(animationId);
	}

	inline constexpr uint32 SjangoAnimationIdCount = ToIndex(SjangoAnimationId::Count);

	inline constexpr std::array<const wchar_t*, SjangoAnimationIdCount> SjangoAnimationClipNames =
	{
		L"ANIM_IDLE",
		L"ANIM_DAMAGE",
	};

	inline std::wstring GetSjangoAnimationClipName(SjangoAnimationId animationId)
	{
		return SjangoAnimationClipNames[ToIndex(animationId)];
	}
}
