#pragma once

#include <iosfwd>
#include <string>

namespace gm
{
	struct EnvironmentObjectData;
	struct MapData;
	struct TriggerSequenceBindingData;

	class BinaryMapLoader
	{
	public:
		static bool Load(const std::wstring& filePath, MapData& outMapData);

	private:
		static bool ReadEnvironmentObject(std::istream& inputStream, EnvironmentObjectData& outObject);
		static bool ReadEnvironmentComponent(std::istream& inputStream, EnvironmentObjectData& outObject);
		static bool ReadTriggerSequenceBinding(std::istream& inputStream, TriggerSequenceBindingData& outBinding);
	};
}
