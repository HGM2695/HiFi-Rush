#include "BinaryModelLoader.h"
#include "BinaryIO.h"
#include "GMAssert.h"
#include "PathUtil.h"
#include "GMLog.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <utility>

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

		if (modelData.type == ModelType::Skeletal)
		{
			GM_ASSERT_RETURN_VAL(ReadBones(inputStream, modelData), modelData, "Skeletal Model Bone 데이터 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadAnimations(inputStream, modelData), modelData, "Skeletal Model Animation 데이터 읽기에 실패했습니다.");
		}

		GM_ASSERT_RETURN_VAL(ReadMeshes(inputStream, modelData), modelData, "모델 Mesh 데이터 읽기에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadMaterials(inputStream, modelData), modelData, "모델 Material 데이터 읽기에 실패했습니다.");

		return modelData;
	}

	/// private Method //////////////////////////////////////////////////////////////////////////////////////////////////////
	bool BinaryModelLoader::ReadBones(std::istream& inputStream, ModelData& modelData)
	{
		uint32 boneCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, boneCount), false, "Bone 개수 읽기에 실패했습니다.");

		modelData.bones.resize(boneCount);

		int i = 0;
		for (BoneData& bone : modelData.bones)
		{
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, bone.name), false, "Bone 이름 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, bone.transform), false, "Bone Transform 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, bone.parentBoneIndex), false, "Bone 부모 인덱스 읽기에 실패했습니다.");

			GM_ASSERT(bone.parentBoneIndex < i, "Bone Idx %d, Parent Idx : %d Parent idx가 항상 작다는 전제를 어기는 케이스가 존재합니다.", i, bone.parentBoneIndex);
			GM_LOG("%d : BoneName : %ls", i, bone.name.c_str());
			++i;

		}

		return true;
	}

	bool BinaryModelLoader::ReadAnimations(std::istream& inputStream, ModelData& modelData)
	{
		uint32 animationCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, animationCount), false, "Animation 개수 읽기에 실패했습니다.");

		modelData.animations.resize(animationCount);
		for (SkeletalAnimationClipData& animationClip : modelData.animations)
		{
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, animationClip.name), false, "Animation 이름 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, animationClip.duration), false, "Animation Duration 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, animationClip.ticksPerSecond), false, "Animation TicksPerSecond 읽기에 실패했습니다.");

			uint32 channelCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, channelCount), false, "Animation Channel 개수 읽기에 실패했습니다.");

			animationClip.channels.resize(channelCount);
			for (AnimationChannelData& channel : animationClip.channels)
			{
				GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, channel.name), false, "Animation Channel 이름 읽기에 실패했습니다.");
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, channel.boneIndex), false, "Animation Channel Bone 인덱스 읽기에 실패했습니다.");

				uint32 keyFrameCount = 0;
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, keyFrameCount), false, "Animation KeyFrame 개수 읽기에 실패했습니다.");
				GM_ASSERT_RETURN_VAL(ReadBinaryVector(inputStream, channel.keyFrames, keyFrameCount), false, "Animation KeyFrame 데이터 읽기에 실패했습니다.");
			}
		}

		return true;
	}

	bool BinaryModelLoader::ReadMesh(std::istream& inputStream, ModelData& modelData)
	{
		std::wstring meshName;
		GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, meshName), false, "Mesh 이름 읽기에 실패했습니다.");

		uint32 materialIndex = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, materialIndex), false, "Mesh Material 인덱스 읽기에 실패했습니다.");

		uint32 vertexCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, vertexCount), false, "Mesh 정점 개수 읽기에 실패했습니다.");

		// Vertex
		uint32 vertexBase = 0;
		if (modelData.type == ModelType::Skeletal)
		{
			vertexBase = static_cast<uint32>(modelData.skinnedVertices.size());
			modelData.skinnedVertices.resize(vertexBase + vertexCount);

			for (uint32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, modelData.skinnedVertices[vertexBase + vertexIndex]), false, "Skeletal Mesh 정점 데이터 읽기에 실패했습니다.");
			}
		}
		else
		{
			vertexBase = static_cast<uint32>(modelData.vertices.size());
			modelData.vertices.resize(vertexBase + vertexCount);

			for (uint32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
			{
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, modelData.vertices[vertexBase + vertexIndex]), false, "Static Mesh 정점 데이터 읽기에 실패했습니다.");
			}
		}

		// section
		MeshSection section{};
		section.name = std::move(meshName);
		section.textureSetIndex = materialIndex;

		if (modelData.type == ModelType::Skeletal)
		{
			uint32 meshBoneCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, meshBoneCount), false, "Skeletal Mesh Bone 개수 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryVector(inputStream, section.boneIndices, meshBoneCount), false, "Skeletal Mesh Bone 인덱스 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryVector(inputStream, section.offsetMatrices, meshBoneCount), false, "Skeletal Mesh Offset Matrix 읽기에 실패했습니다.");
		}

		// index
		uint32 indexCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, indexCount), false, "Mesh 인덱스 개수 읽기에 실패했습니다.");

		section.indexStart = static_cast<uint32>(modelData.indices.size());
		section.indexCount = indexCount;
		modelData.indices.resize(section.indexStart + indexCount);

		for (uint32 index = 0; index < indexCount; ++index)
		{
			uint32 localIndex = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, localIndex), false, "Mesh 인덱스 데이터 읽기에 실패했습니다.");
			modelData.indices[section.indexStart + index] = vertexBase + localIndex;
		}

		modelData.sections.push_back(std::move(section));
		return true;
	}

	bool BinaryModelLoader::ReadMeshes(std::istream& inputStream, ModelData& modelData)
	{
		uint32 meshCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, meshCount), false, "Mesh 개수 읽기에 실패했습니다.");

		for (uint32 meshIndex = 0; meshIndex < meshCount; ++meshIndex)
		{
			GM_ASSERT_RETURN_VAL(ReadMesh(inputStream, modelData), false, "Mesh 데이터 읽기에 실패했습니다.");
		}

		return true;
	}

	bool BinaryModelLoader::ReadMaterials(std::istream& inputStream, ModelData& modelData)
	{
		uint32 materialCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, materialCount), false, "Material 개수 읽기에 실패했습니다.");

		modelData.textureSets.resize(materialCount);
		for (uint32 materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			MeshTextureSet& textureSet = modelData.textureSets[materialIndex];

			uint32 totalTextureCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, totalTextureCount), false, "Material Texture 개수 읽기에 실패했습니다.");
			if (totalTextureCount == 0)
				continue;

			std::array<uint32, LegacyTextureTypeCount> textureCounts{};
			for (uint32& textureCount : textureCounts)
				GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, textureCount), false, "Texture 타입별 개수 읽기에 실패했습니다.");

			for (uint32 textureType = 0; textureType < LegacyTextureTypeCount; ++textureType)
			{
				for (uint32 textureIndex = 0; textureIndex < textureCounts[textureType]; ++textureIndex)
				{
					std::string texturePath;
					GM_ASSERT_RETURN_VAL(ReadBinaryString(inputStream, texturePath), false, "Texture 경로 읽기에 실패했습니다.");

					const TextureSlot engineSlot = ToEngineTextureSlot(textureType);
					if (engineSlot == TextureSlot::Count)
						continue;

					std::wstring& targetKey = textureSet.textureKeys[ToTexturelSlotIndex(engineSlot)];
					targetKey = Utf8ToWide(GetFileNameWithoutExtension(texturePath).c_str());
				}
			}
		}

		return true;
	}
}
