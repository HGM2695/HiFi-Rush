#pragma once

#include <array>

namespace gm
{
	struct ChiEffectSocketBinding
	{
		const wchar_t* socketName = nullptr;
		const wchar_t* boneName = nullptr;
	};

	inline constexpr wchar_t ChiSkeletalMeshResourceKey[] = L"chi";
	inline constexpr wchar_t ChiDefaultAnimationResourceKey[] = L"chi.DefaultAnimation";
	inline constexpr wchar_t ChiGuitarResourceKey[] = L"chi.Guitar";
	inline constexpr wchar_t ChiFingerSnapSocketName[] = L"Player.Effect.LeftMiddleFinger";
	inline constexpr std::array ChiEffectSocketBindings
	{
		ChiEffectSocketBinding{ L"Player.Effect.LeftToe", L"l_toe" },
		ChiEffectSocketBinding{ L"Player.Effect.RightToe", L"r_toe" },
		ChiEffectSocketBinding{ L"Player.Effect.LeftShin", L"l_shin" },
		ChiEffectSocketBinding{ L"Player.Effect.RightShin", L"r_shin" },
		ChiEffectSocketBinding{ L"Player.Effect.LeftForearm", L"l_forearm" },
		ChiEffectSocketBinding{ L"Player.Effect.RightForearm", L"r_forearm" },
		ChiEffectSocketBinding{ L"Player.Effect.Back", L"shirts_back_01" },
		ChiEffectSocketBinding{ ChiFingerSnapSocketName, L"l_middle_03" }
	};
}
