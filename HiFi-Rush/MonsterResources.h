#pragma once

#include "MonsterTypes.h"

#include <string>

namespace gm
{
	struct MonsterResourceInfo
	{
		const wchar_t*	resourceKey = nullptr;
		const wchar_t*	modelFileName = nullptr;
		uint32			defaultAnimationIndex = 0;
	};

	const MonsterResourceInfo*	FindMonsterResourceInfo(MonsterType type);
	std::wstring				GetMonsterAnimationResourceKey(MonsterType type, uint32 animationIndex);
	std::wstring				GetMonsterDefaultAnimationResourceKey(MonsterType type);
}
