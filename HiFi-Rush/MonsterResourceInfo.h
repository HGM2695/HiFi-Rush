#pragma once

#include "MonsterTypes.h"

#include <string>

namespace gm
{
	struct MonsterResourceInfo
	{
		const wchar_t*	commonResourceKey = nullptr;
		const wchar_t*	modelFileName = nullptr;
		uint32			defaultAnimationIndex = 0;
		const wchar_t*	weaponResourceKey = nullptr;
		const wchar_t*	weaponModelFileName = nullptr;
	};

	const MonsterResourceInfo*	GetMonsterResourceInfo(MonsterType type);
	std::wstring				GetMonsterAnimationClipKey(MonsterType type, uint32 animationIndex);
	std::wstring				GetMonsterDefaultAnimationClipKey(MonsterType type);
}
