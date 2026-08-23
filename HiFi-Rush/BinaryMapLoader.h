#pragma once

#include <iosfwd>
#include <string>

namespace gm
{
	struct EnvironmentObjectData;
	struct DepthFogSettings;
	struct MapData;
	struct MonsterSpawnData;
	struct SceneAmbientSettings;
	struct ToneMappingSettings;
	struct TriggerBindingData;

	class BinaryMapLoader
	{
	public:
		static bool Load(const std::wstring& filePath, MapData& outMapData);

	private:
		static bool ReadEnvironmentObject(std::istream& inputStream, EnvironmentObjectData& outObject);
		static bool ReadEnvironmentComponent(std::istream& inputStream, EnvironmentObjectData& outObject);
		static bool ReadMonsterSpawn(std::istream& inputStream, MonsterSpawnData& outSpawnData);
		static bool ReadSceneAmbientSettings(std::istream& inputStream, SceneAmbientSettings& outSettings);
		static bool ReadDepthFogSettings(std::istream& inputStream, DepthFogSettings& outSettings);
		static bool ReadToneMappingSettings(std::istream& inputStream, ToneMappingSettings& outSettings);
		static bool ReadTriggerBinding(std::istream& inputStream, TriggerBindingData& outBinding);
	};
}
