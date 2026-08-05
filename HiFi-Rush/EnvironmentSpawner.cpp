#include "EnvironmentSpawner.h"
#include "Application.h"
#include "BeatAudioLevelMoveComponent.h"
#include "BeatMoveComponent.h"
#include "BeatOrbitComponent.h"
#include "BeatStaticMeshCycleComponent.h"
#include "BeatPositionSequenceComponent.h"
#include "BeatTransformComponent.h"
#include "BeatTriggeredRotationComponent.h"
#include "EnvironmentMapTypes.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "GMLog.h"
#include "HiFiRushGameInstance.h"
#include "MathUtil.h"
#include "Random.h"
#include "Resources.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"

#include <string>

namespace gm
{
	namespace
	{
		std::wstring GetEnvironmentModelKey(uint32 modelIndex)
		{
			return L"Environment" + std::to_wstring(modelIndex);
		}

		const BeatSystem& GetBeatSystem()
		{
			const HiFiRushGameInstance& gameInstance = static_cast<const HiFiRushGameInstance&>(APPLICATION.GetGameInstance());
			return gameInstance.GetBeatSystem();
		}

		bool IsAudienceModel(uint32 modelIndex)
		{
			return modelIndex >= 29 && modelIndex <= 36;
		}

		bool IsBeatOrbitModel(uint32 modelIndex)
		{
			return modelIndex >= 174 && modelIndex <= 178;
		}

		bool IsBeatMoveModel(uint32 modelIndex)
		{
			return (modelIndex >= 4 && modelIndex <= 6) || modelIndex == 23 || modelIndex == 24 || modelIndex == 163;
		}

		bool IsBeatAudioLevelMoveModel(uint32 modelIndex)
		{
			return modelIndex == 164;
		}

		bool IsBeatTriggeredRotationModel(uint32 modelIndex)
		{
			return modelIndex == 157;
		}

		bool IsBeatPositionSequenceModel(uint32 modelIndex)
		{
			return modelIndex == 158;
		}

		float CreateBeatAudioLevelMoveDistance()
		{
			return 5.f + static_cast<float>(Math::RandomInt(0, 60)) * 0.1f;
		}

		bool TryConfigureBeatTransform(const EnvironmentObjectData& objectData, TransformComponent& transform, BeatTransformDesc& outDesc)
		{
			if (objectData.modelIndex == 9)
			{
				transform.SetScale(Vector3{ 1.f, 1.5f, 1.3f });
				outDesc.type = BeatTransformType::ScaleMultiplier;
				outDesc.maxScaleMultiplier = Vector3{ 1.f, 7.f / 6.f, 1.f };
				outDesc.cycleBeats = 2.f;
				return true;
			}

			if (objectData.modelIndex >= 182 && objectData.modelIndex <= 184)
			{
				Vector3 initialScale = transform.GetScale();
				initialScale.y = 0.5f;
				transform.SetScale(initialScale);

				outDesc.type = BeatTransformType::ScaleMultiplier;
				outDesc.maxScaleMultiplier = Vector3{ 1.f, 4.f, 1.f };
				outDesc.cycleBeats = 1.f;
				return true;
			}

			if (objectData.moveBeat == 0)
				return false;

			if (objectData.modelIndex >= 29 && objectData.modelIndex <= 36)
			{
				outDesc.type = BeatTransformType::PositionOffset;
				outDesc.positionOffset = Vector3{ 0.f, 1.f, 0.f };
				outDesc.cycleBeats = static_cast<float>(objectData.moveBeat);
				return true;
			}

			if (objectData.modelIndex >= 120 && objectData.modelIndex <= 123)
			{
				outDesc.type = BeatTransformType::ScaleMultiplier;
				outDesc.maxScaleMultiplier = Vector3{ 1.2f, 1.f, 1.2f };
				outDesc.cycleBeats = static_cast<float>(objectData.moveBeat);
				return true;
			}

			return false;
		}
	}

	EnvironmentSpawner::EnvironmentSpawner(Resources& resources)
		: _resources(resources)
	{}

	bool EnvironmentSpawner::Spawn(Scene& scene, const EnvironmentMapData& mapData) const
	{
		return SpawnObjects(scene, mapData, false);
	}

	bool EnvironmentSpawner::SpawnTriggerObjects(Scene& scene, const EnvironmentMapData& mapData) const
	{
		return SpawnObjects(scene, mapData, true);
	}

	bool EnvironmentSpawner::SpawnObjects(Scene& scene, const EnvironmentMapData& mapData, bool isTriggerObject) const
	{
		for (const EnvironmentObjectData& objectData : mapData.objects)
		{
			if (SpawnObject(scene, objectData, isTriggerObject) == false)
				return false;
		}

		GM_LOG("Environment objects spawned. count=%zu", mapData.objects.size());
		return true;
	}

	bool EnvironmentSpawner::SpawnObject(Scene& scene, const EnvironmentObjectData& objectData, bool isTriggerObject) const
	{
		const std::wstring modelKey = GetEnvironmentModelKey(objectData.modelIndex);
		const std::shared_ptr<StaticMesh> staticMesh = _resources.Find<StaticMesh>(modelKey);
		const std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(modelKey);
		GM_ASSERT_RETURN_VAL(staticMesh || skeletalMesh, false, "환경 오브젝트가 참조하는 모델 리소스가 없습니다. key=%ls", modelKey.c_str());

		GameObject* gameObject = scene.SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN_VAL(gameObject, false, "환경 오브젝트 생성에 실패했습니다. key=%ls", modelKey.c_str());
		TransformComponent* transform = gameObject->GetTransform();
		transform->SetWorldMatrix(objectData.world);

		if (isTriggerObject && IsBeatMoveModel(objectData.modelIndex))
		{
			BeatMoveDesc desc{};
			desc.targetPosition = objectData.moveEndPosition;
			BeatMoveComponent* beatMove = gameObject->AddComponent<BeatMoveComponent>(GetBeatSystem(), desc);
			GM_ASSERT_RETURN_VAL(beatMove, false, "BeatMoveComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		}

		if (isTriggerObject && IsBeatTriggeredRotationModel(objectData.modelIndex))
		{
			const Vector3 lookDirection = Math::GetLookVector(transform->GetRotation());
			BeatTriggeredRotationDesc desc{};
			desc.angleDegrees = lookDirection.x > 0.f ? -90.f : 90.f;
			BeatTriggeredRotationComponent* beatRotation = gameObject->AddComponent<BeatTriggeredRotationComponent>(GetBeatSystem(), desc);
			GM_ASSERT_RETURN_VAL(beatRotation, false, "BeatTriggeredRotationComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		}

		if (isTriggerObject && IsBeatPositionSequenceModel(objectData.modelIndex))
		{
			const float moveHeight = objectData.moveEndPosition.y - transform->GetY();
			BeatPositionSequenceDesc desc{};
			desc.positionOffsets = {
				Vector3{ 0.f, 0.f, 0.f },
				Vector3{ 0.f, moveHeight * (1.f / 3.f), 0.f },
				Vector3{ 0.f, moveHeight * (2.f / 3.f), 0.f },
				Vector3{ 0.f, moveHeight, 0.f },
				Vector3{ 0.f, moveHeight, 0.f },
				Vector3{ 0.f, moveHeight * (2.f / 3.f), 0.f },
				Vector3{ 0.f, moveHeight * (1.f / 3.f), 0.f },
				Vector3{ 0.f, 0.f, 0.f }
			};
			BeatPositionSequenceComponent* positionSequence = gameObject->AddComponent<BeatPositionSequenceComponent>(GetBeatSystem(), desc);
			GM_ASSERT_RETURN_VAL(positionSequence, false, "BeatPositionSequenceComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		}

		BeatTransformDesc beatTransformDesc{};
		if (TryConfigureBeatTransform(objectData, *transform, beatTransformDesc))
		{
			BeatTransformComponent* beatTransform = gameObject->AddComponent<BeatTransformComponent>(GetBeatSystem(), beatTransformDesc);
			GM_ASSERT_RETURN_VAL(beatTransform, false, "BeatTransformComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		}

		if (IsBeatOrbitModel(objectData.modelIndex))
		{
			BeatOrbitComponent* beatOrbit = gameObject->AddComponent<BeatOrbitComponent>(GetBeatSystem(), BeatOrbitDesc{ .faceCenter = true });
			GM_ASSERT_RETURN_VAL(beatOrbit, false, "BeatOrbitComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		}

		if (IsBeatAudioLevelMoveModel(objectData.modelIndex))
		{
			BeatAudioLevelMoveDesc desc{};
			desc.maxDistance = CreateBeatAudioLevelMoveDistance();
			BeatAudioLevelMoveComponent* audioLevelMove = gameObject->AddComponent<BeatAudioLevelMoveComponent>(GetBeatSystem(), desc);
			GM_ASSERT_RETURN_VAL(audioLevelMove, false, "BeatAudioLevelMoveComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		}

		if (staticMesh)
		{
			StaticMeshComponent* meshComponent = gameObject->AddComponent<StaticMeshComponent>();
			GM_ASSERT_RETURN_VAL(meshComponent, false, "StaticMeshComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
			meshComponent->SetStaticMesh(staticMesh);

			if (IsAudienceModel(objectData.modelIndex))
			{
				const uint32 firstVariantIndex = objectData.modelIndex <= 32 ? 29 : 33;
				std::vector<std::shared_ptr<StaticMesh>> meshVariants;
				meshVariants.reserve(4);
				for (uint32 variantIndex = firstVariantIndex; variantIndex < firstVariantIndex + 4; ++variantIndex)
				{
					const std::wstring variantKey = GetEnvironmentModelKey(variantIndex);
					std::shared_ptr<StaticMesh> variant = _resources.Find<StaticMesh>(variantKey);
					GM_ASSERT_RETURN_VAL(variant, false, "관객 StaticMesh 리소스가 없습니다. key=%ls", variantKey.c_str());
					meshVariants.push_back(std::move(variant));
				}

				BeatStaticMeshCycleComponent* meshCycle = gameObject->AddComponent<BeatStaticMeshCycleComponent>(GetBeatSystem(), *meshComponent, std::move(meshVariants));
				GM_ASSERT_RETURN_VAL(meshCycle, false, "BeatStaticMeshCycleComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
			}

			return true;
		}

		SkeletalMeshComponent* meshComponent = gameObject->AddComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN_VAL(meshComponent, false, "SkeletalMeshComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
		meshComponent->SetSkeletalMesh(skeletalMesh);

		const std::shared_ptr<SkeletalAnimationClip> defaultClip = _resources.Find<SkeletalAnimationClip>(modelKey + L".DefaultAnimation");
		if (defaultClip)
		{
			SkeletalAnimatorComponent* animator = gameObject->AddComponent<SkeletalAnimatorComponent>();
			GM_ASSERT_RETURN_VAL(animator, false, "SkeletalAnimatorComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
			GM_ASSERT_RETURN_VAL(animator->AddClip(L"Default", defaultClip), false, "환경 오브젝트의 기본 애니메이션 등록에 실패했습니다. key=%ls", modelKey.c_str());
			GM_ASSERT_RETURN_VAL(animator->Play(L"Default"), false, "환경 오브젝트의 기본 애니메이션 재생에 실패했습니다. key=%ls", modelKey.c_str());
		}

		return true;
	}
}
