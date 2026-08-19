#pragma once

#include <iosfwd>
#include <string>

namespace gm
{
	struct EnvironmentObjectData;
	struct MapData;
	struct MonsterSpawnData;
	struct TriggerBindingData;

	class BinaryMapLoader
	{
	public:
		static bool Load(const std::wstring& filePath, MapData& outMapData);

	private:
		static bool ReadEnvironmentObject(std::istream& inputStream, EnvironmentObjectData& outObject);
		static bool ReadEnvironmentComponent(std::istream& inputStream, EnvironmentObjectData& outObject);
		static bool ReadMonsterSpawn(std::istream& inputStream, MonsterSpawnData& outSpawnData);
		static bool ReadTriggerBinding(std::istream& inputStream, TriggerBindingData& outBinding);
	};
}
