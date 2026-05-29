#include "BinaryModelLoader.h"
#include "BinaryIO.h"
#include "GMAssert.h"
#include "PathUtil.h"

#include <array>
#include <filesystem>
#include <fstream>

namespace gm
{
	namespace
	{
		constexpr uint32 LegacyTextureTypeCount = 18;
		constexpr uint32 LegacyDiffuseTextureType = 1;
		constexpr uint32 LegacyNormalsTextureType = 6;
		constexpr uint32 LegacyEmissiveTextureType = 4;
		constexpr uint32 LegacyMetalnessTextureType = 15;
		constexpr uint32 LegacyRoughnessTextureType = 16;
		constexpr uint32 LegacyAmbientOcclusionTextureType = 17;

		TextureSlot ToEngineTextureSlot(uint32 legacyTextureType)
		{
			switch (legacyTextureType)
			{
			case LegacyDiffuseTextureType:
				return TextureSlot::BaseColor;
			case LegacyNormalsTextureType:
				return TextureSlot::Normal;
			case LegacyEmissiveTextureType:
				return TextureSlot::Emissive;
			case LegacyMetalnessTextureType:
				return TextureSlot::Metallic;
			case LegacyRoughnessTextureType:
				return TextureSlot::Roughness;
			case LegacyAmbientOcclusionTextureType:
				return TextureSlot::AmbientOcclusion;
			default:
				return TextureSlot::Count;
			}
		}

	}

	ModelData BinaryModelLoader::Load(const std::wstring& filepath)
	{
		std::ifstream inputStream(std::filesystem::path(filepath), std::ios::binary);

		ModelData modelData{};
		GM_ASSERT_RETURN_VAL(inputStream.is_open(), modelData, "모델 파일 열기에 실패했습니다. path=%ls", filepath.c_str());

		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, modelData.type), modelData, "모델 타입 읽기에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, modelData.preTransform), modelData, "모델 PreTransform 읽기에 실패했습니다.");

		uint32 meshCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, meshCount), modelData, "메쉬 개수 읽기에 실패했습니다.");

		for (uint32 meshIndex = 0; meshIndex < meshCount; ++meshIndex)
		{
			std::string meshName;
			GM_ASSERT_RETURN_VAL(ReadBinaryString(inputStream, meshName), modelData, "메쉬 이름 읽기에 실패했습니다.");

			uint32 materialIndex = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, materialIndex), modelData, "메쉬 Material 인덱스 읽기에 실패했습니다.");

			uint32 vertexCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, vertexCount), modelData, "메쉬 정점 개수 읽기에 실패했습니다.");

			const uint32 vertexBase = static_cast<uint32>(modelData.vertices.size());
			for (uint32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				VertexMesh vertex{};
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, vertex), modelData, "메쉬 정점 데이터 읽기에 실패했습니다.");
				modelData.vertices.push_back(vertex);
			}

			uint32 indexCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, indexCount), modelData, "메쉬 인덱스 개수 읽기에 실패했습니다.");

			MeshSection section{};
			section.name = Utf8ToWide(meshName.c_str());
			section.indexStart = static_cast<uint32>(modelData.indices.size());
			section.indexCount = indexCount;
			section.textureSlotIndex = materialIndex;

			for (uint32 index = 0; index < indexCount; ++index)
			{
				uint32 localIndex = 0;
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, localIndex), modelData, "메쉬 인덱스 데이터 읽기에 실패했습니다.");
				modelData.indices.push_back(vertexBase + localIndex);
			}

			modelData.sections.push_back(section);
		}

		uint32 materialCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, materialCount), modelData, "Material 개수 읽기에 실패했습니다.");

		modelData.textureSlots.resize(materialCount);
		for (uint32 materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			MeshTextureSlot& textureSlot = modelData.textureSlots[materialIndex];

			uint32 totalTextureCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, totalTextureCount), modelData, "Material Texture 개수 읽기에 실패했습니다.");
			if (totalTextureCount == 0)
				continue;

			std::array<uint32, LegacyTextureTypeCount> textureCounts{};
			for (uint32& textureCount : textureCounts)
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, textureCount), modelData, "Texture 타입별 개수 읽기에 실패했습니다.");

			for (uint32 textureType = 0; textureType < LegacyTextureTypeCount; ++textureType)
			{
				for (uint32 textureIndex = 0; textureIndex < textureCounts[textureType]; ++textureIndex)
				{
					std::string texturePath;
					GM_ASSERT_RETURN_VAL(ReadBinaryString(inputStream, texturePath), modelData, "Texture 경로 읽기에 실패했습니다.");

					std::wstring textureKey = Utf8ToWide(GetFileNameWithoutExtension(texturePath).c_str());
					const TextureSlot engineSlot = ToEngineTextureSlot(textureType);
					if (engineSlot == TextureSlot::Count)
						continue;

					std::wstring& targetKey = textureSlot.textureKeys[ToTexturelSlotIndex(engineSlot)];
					targetKey = textureKey;
				}
			}
		}

		return modelData;
	}
}
