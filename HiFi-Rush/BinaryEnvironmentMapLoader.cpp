#include "BinaryEnvironmentMapLoader.h"
#include "BinaryIO.h"
#include "EnvironmentMapTypes.h"
#include "GMAssert.h"

#include <filesystem>
#include <fstream>

namespace gm
{
	namespace
	{
		constexpr uint32 MaxEnvironmentObjectCount = 100000;

		bool ReadEnvironmentObject(std::istream& inputStream, EnvironmentObjectData& outObject)
		{
			uint32 renderType = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, renderType), false, "환경 오브젝트의 렌더 타입을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(
				renderType == static_cast<uint32>(EnvironmentRenderType::Opaque) ||
				renderType == static_cast<uint32>(EnvironmentRenderType::InOrderBlend) ||
				renderType == static_cast<uint32>(EnvironmentRenderType::AfterEdge),
				false,
				"지원하지 않는 환경 오브젝트 렌더 타입입니다. type=%u",
				renderType);

			outObject.renderType = static_cast<EnvironmentRenderType>(renderType);
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.modelIndex), false, "환경 오브젝트의 모델 인덱스를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.moveBeat), false, "환경 오브젝트의 이동 비트를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.colorVariant), false, "환경 오브젝트의 색상 타입을 읽는 데 실패했습니다.");

			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.world), false, "환경 오브젝트의 월드 행렬을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.moveEndPosition), false, "환경 오브젝트의 이동 종료 위치를 읽는 데 실패했습니다.");
			return true;
		}
	}

	bool BinaryEnvironmentMapLoader::Load(const std::wstring& filePath, EnvironmentMapData& outMapData)
	{
		std::ifstream inputStream(std::filesystem::path(filePath), std::ios::binary);
		GM_ASSERT_RETURN_VAL(inputStream.is_open(), false, "환경 맵 바이너리 파일을 열지 못했습니다. path=%ls", filePath.c_str());

		uint32 objectCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, objectCount), false, "환경 오브젝트 개수를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(objectCount <= MaxEnvironmentObjectCount, false, "환경 오브젝트 개수가 허용 범위를 초과했습니다. count=%u", objectCount);

		EnvironmentMapData loadedData{};
		loadedData.objects.resize(objectCount);
		for (EnvironmentObjectData& object : loadedData.objects)
			GM_ASSERT_RETURN_VAL(ReadEnvironmentObject(inputStream, object), false, "환경 오브젝트 데이터를 읽는 데 실패했습니다.");

		outMapData = std::move(loadedData);
		return true;
	}
}
