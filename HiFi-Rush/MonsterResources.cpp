#include "MonsterResources.h"

#include <array>

namespace gm
{
	namespace
	{
		constexpr std::array<MonsterResourceInfo, static_cast<size_t>(MonsterType::Count)> MonsterResourceInfos =
		{
			MonsterResourceInfo{ L"Monster.Sjango", L"Sjango.bin", 0 },
			MonsterResourceInfo{ L"Monster.Sword", L"Sword.bin", 0 },
			MonsterResourceInfo{ L"Monster.Gunner", L"Gunner.bin", 13 },
		};
	}

	const MonsterResourceInfo* FindMonsterResourceInfo(MonsterType type)
	{
		const size_t index = static_cast<size_t>(type);
		if (index >= MonsterResourceInfos.size())
			return nullptr;

		return &MonsterResourceInfos[index];
	}

	std::wstring GetMonsterAnimationResourceKey(MonsterType type, uint32 animationIndex)
	{
		const MonsterResourceInfo* info = FindMonsterResourceInfo(type);
		if (info == nullptr)
			return {};

		return std::wstring(info->resourceKey) + L".Animation" + std::to_wstring(animationIndex);
	}

	std::wstring GetMonsterDefaultAnimationResourceKey(MonsterType type)
	{
		const MonsterResourceInfo* info = FindMonsterResourceInfo(type);
		if (info == nullptr)
			return {};

		return std::wstring(info->resourceKey) + L".DefaultAnimation";
	}
}
