#pragma once

#include "QamilAnimationTypes.h"

#include <array>
#include <string>

namespace gm
{
	inline constexpr wchar_t			QamilSkeletalMeshResourceKey[] = L"Boss.Qamil";
	inline constexpr wchar_t			QamilModelFileName[] = L"Qamil.bin";
	inline constexpr wchar_t			QamilMissileSkeletalMeshResourceKey[] = L"Boss.Qamil.Missile";
	inline constexpr wchar_t			QamilMissileAnimationResourceKey[] = L"Boss.Qamil.Missile.Animation";
	inline constexpr wchar_t			QamilMissileWarningTextureResourceKey[] = L"Red_Circle";
	inline constexpr wchar_t			QamilPhase3BodyTextureResourceKey[] = L"T_em1000_base_Red_noalpha_C";
	inline constexpr wchar_t			QamilMissileModelFileName[] = L"Missile.bin";
	inline constexpr QamilAnimationId	QamilDefaultAnimationId = QamilAnimationId::Idle;
	inline constexpr std::array<uint32, 10> QamilPhase3BodySectionIndices{ 0, 1, 2, 5, 17, 19, 21, 23, 26, 27 };

	struct QamilMissileSocketBinding
	{
		const wchar_t* launchSocketName = nullptr;
		const wchar_t* launchBoneName = nullptr;
		const wchar_t* directionSourceSocketName = nullptr;
		const wchar_t* directionSourceBoneName = nullptr;
	};

	inline constexpr std::array<QamilMissileSocketBinding, 4> QamilMissileSocketBindings =
	{{
		{ L"Qamil.Missile.LeftA.Launch", L"l_missile_a_03", L"Qamil.Missile.LeftA.DirectionSource", L"l_missile_a_bullet_00" },
		{ L"Qamil.Missile.LeftB.Launch", L"l_missile_b_03", L"Qamil.Missile.LeftB.DirectionSource", L"l_missile_b_bullet_00" },
		{ L"Qamil.Missile.RightA.Launch", L"r_missile_a_03", L"Qamil.Missile.RightA.DirectionSource", L"r_missile_a_bullet_00" },
		{ L"Qamil.Missile.RightB.Launch", L"r_missile_b_03", L"Qamil.Missile.RightB.DirectionSource", L"r_missile_b_bullet_00" },
	}};

	inline std::wstring GetQamilAnimationClipKey(QamilAnimationId animationId)
	{
		return std::wstring(QamilSkeletalMeshResourceKey) + L".Animation" + std::to_wstring(ToIndex(animationId));
	}
}
