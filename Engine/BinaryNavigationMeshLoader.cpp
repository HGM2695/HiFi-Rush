#include "BinaryNavigationMeshLoader.h"

#include "BinaryIO.h"
#include "GMAssert.h"

#include <fstream>

namespace gm
{
	namespace
	{
		NavigationCellType ToNavigationCellType(int32 value)
		{
			switch (value)
			{
			case 0:
				return NavigationCellType::Normal;
			case 1:
				return NavigationCellType::Jump;
			case 2:
				return NavigationCellType::Dead;
			case 3:
				return NavigationCellType::Dummy;
			default:
				GM_ASSERT_RETURN_VAL(false, NavigationCellType::Dummy, "지원하지 않는 NavigationCellType입니다.");
			}
		}
	}

	bool BinaryNavigationMeshLoader::Load(const std::wstring& filePath, _Out_ NavigationMeshData& outData)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		GM_ASSERT_RETURN_VAL(inputStream.is_open(), false, "NavigationMesh 파일을 열 수 없습니다.");

		uint32 cellCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, cellCount), false, "NavigationMesh Cell 개수 읽기에 실패했습니다.");

		NavigationMeshData data{};
		data.cells.resize(cellCount);

		for (uint32 cellIndex = 0; cellIndex < cellCount; ++cellIndex)
		{
			NavigationCellData& cell = data.cells[cellIndex];
			GM_ASSERT_RETURN_VAL(ReadBinaryArray(inputStream, cell.points), false, "NavigationMesh Cell 정점 읽기에 실패했습니다.");

			uint32 fileCellIndex = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, fileCellIndex), false, "NavigationMesh Cell 인덱스 읽기에 실패했습니다.");

			GM_ASSERT_RETURN_VAL(ReadBinaryArray(inputStream, cell.neighborIndices), false, "NavigationMesh 이웃 인덱스 읽기에 실패했습니다.");

			int32 cellType = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, cellType), false, "NavigationMesh Cell 타입 읽기에 실패했습니다.");
			cell.type = ToNavigationCellType(cellType);

			for (uint32 edgeIndex = 0; edgeIndex < NavigationCellData::EdgeCount; ++edgeIndex)
			{
				int32 neighborType = 0;
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, neighborType), false, "NavigationMesh 이웃 Cell 타입 읽기에 실패했습니다.");
				cell.neighborTypes[edgeIndex] = ToNavigationCellType(neighborType);
			}
		}

		outData = std::move(data);
		return true;
	}
}
