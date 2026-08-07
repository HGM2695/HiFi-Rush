#include "EnvironmentSpawner.h"
#include "BeatTriggerSequenceComponent.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "GMLog.h"
#include "HiFiRushStatics.h"
#include "MapTypes.h"
#include "Resources.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"

#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>

namespace gm
{
	namespace
	{
		std::wstring GetEnvironmentModelKey(uint32 modelIndex)
		{
			return L"Environment" + std::to_wstring(modelIndex);
		}
	}

	EnvironmentSpawner::EnvironmentSpawner(Resources& resources)
		: _resources(resources), _componentFactory(resources, HiFiRushStatics::GetBeatSystem())
	{}

	bool EnvironmentSpawner::Spawn(Scene& scene, const MapData& mapData) const
	{
		std::vector<SpawnEntry> spawnEntries;
		spawnEntries.reserve(mapData.objects.size());
		for (const EnvironmentObjectData& objectData : mapData.objects)
		{
			SpawnEntry spawnEntry{};
			if (SpawnObject(scene, objectData, spawnEntry) == false)
				return false;

			spawnEntries.push_back(std::move(spawnEntry));
		}

		GM_ASSERT_RETURN_VAL(BuildTriggerSequences(scene, spawnEntries), false, "환경 오브젝트의 트리거 시퀀스 구성에 실패했습니다.");
		GM_LOG("Environment objects spawned. count=%zu", mapData.objects.size());
		return true;
	}

	bool EnvironmentSpawner::BuildTriggerSequences(Scene& scene, const std::vector<SpawnEntry>& spawnEntries) const
	{
		std::unordered_map<std::wstring, BeatTriggerSequenceComponent*> sequences;
		for (const SpawnEntry& spawnEntry : spawnEntries)
		{
			GM_ASSERT_RETURN_VAL(spawnEntry.owner.IsValid(), false, "환경 오브젝트 Owner가 유효하지 않습니다.");
			for (const EnvironmentTriggerAction& triggerAction : spawnEntry.triggerActions)
			{
				const TriggerSequenceBindingData& binding = triggerAction.triggerBindingData;
				GM_ASSERT_RETURN_VAL(triggerAction.action, false, "환경 오브젝트에 유효하지 않은 Trigger Action이 있습니다. sequenceId=%ls", binding.sequenceId.c_str());
				GM_ASSERT_RETURN_VAL(binding.sequenceId.empty() == false, false, "Trigger Action의 Sequence ID가 비어 있습니다.");
				GM_ASSERT_RETURN_VAL(std::isfinite(binding.beatOffset) && binding.beatOffset >= 0.f, false, "Trigger Action의 Beat Offset이 유효하지 않습니다. sequenceId=%ls", binding.sequenceId.c_str());

				BeatTriggerSequenceComponent*& sequence = sequences[binding.sequenceId];
				if (sequence == nullptr)
				{
					GameObject* sequenceObject = scene.SpawnGameObject<GameObject>();
					GM_ASSERT_RETURN_VAL(sequenceObject, false, "트리거 시퀀스 GameObject 생성에 실패했습니다. sequenceId=%ls", binding.sequenceId.c_str());
					sequence = sequenceObject->AddComponent<BeatTriggerSequenceComponent>(HiFiRushStatics::GetBeatSystem());
					GM_ASSERT_RETURN_VAL(sequence, false, "BeatTriggerSequenceComponent 생성에 실패했습니다. sequenceId=%ls", binding.sequenceId.c_str());
				}

				sequence->AddAction(binding.beatOffset, spawnEntry.owner, *triggerAction.action);
			}
		}

		return true;
	}

	bool EnvironmentSpawner::SpawnObject(Scene& scene, const EnvironmentObjectData& objectData, SpawnEntry& outSpawnEntry) const
	{
		const std::wstring modelKey = GetEnvironmentModelKey(objectData.modelIndex);
		const std::shared_ptr<StaticMesh> staticMesh = _resources.Find<StaticMesh>(modelKey);
		const std::shared_ptr<SkeletalMesh> skeletalMesh = _resources.Find<SkeletalMesh>(modelKey);
		GM_ASSERT_RETURN_VAL(staticMesh || skeletalMesh, false, "환경 오브젝트가 참조하는 모델 리소스가 없습니다. key=%ls", modelKey.c_str());

		GameObject* gameObject = scene.SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN_VAL(gameObject, false, "환경 오브젝트 생성에 실패했습니다. key=%ls", modelKey.c_str());
		gameObject->GetTransform()->SetWorldMatrix(objectData.world);

		if (staticMesh)
		{
			StaticMeshComponent* meshComponent = gameObject->AddComponent<StaticMeshComponent>();
			GM_ASSERT_RETURN_VAL(meshComponent, false, "StaticMeshComponent 생성에 실패했습니다. key=%ls", modelKey.c_str());
			meshComponent->SetStaticMesh(staticMesh);
		}
		else // skeletalMesh
		{
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
		}

		std::vector<EnvironmentTriggerAction> triggerActions;
		GM_ASSERT_RETURN_VAL(_componentFactory.AddComponents(*gameObject, objectData.components, triggerActions), false, "환경 오브젝트 Component 구성에 실패했습니다. key=%ls", modelKey.c_str());
		outSpawnEntry = SpawnEntry{ gameObject->GetWeakPtr(), std::move(triggerActions) };

		return true;
	}
}
