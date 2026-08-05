#include "EnvironmentSpawner.h"
#include "Application.h"
#include "BeatMoveComponent.h"
#include "EnvironmentMapTypes.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "GMLog.h"
#include "HiFiRushGameInstance.h"
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

		bool IsBeatMoveModel(uint32 modelIndex)
		{
			return (modelIndex >= 4 && modelIndex <= 6) || modelIndex == 23 || modelIndex == 24 || modelIndex == 163;
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
		if (staticMesh)
		{
			StaticMeshComponent* meshComponent = gameObject->AddComponent<StaticMeshComponent>();
			GM_ASSERT_RETURN_VAL(meshComponent, false, "StaticMeshComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
			meshComponent->SetStaticMesh(staticMesh);
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
