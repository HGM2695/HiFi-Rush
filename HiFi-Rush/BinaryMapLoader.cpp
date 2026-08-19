#include "BinaryMapLoader.h"
#include "BinaryIO.h"
#include "EnvironmentComponentTypes.h"
#include "GMAssert.h"
#include "MapTypes.h"

#include <cmath>
#include <filesystem>
#include <fstream>

namespace gm
{
	bool BinaryMapLoader::ReadEnvironmentObject(std::istream& inputStream, EnvironmentObjectData& outObject)
	{
		uint32 renderType = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, renderType), false, "환경 오브젝트의 렌더 타입을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(
			renderType == static_cast<uint32>(EnvironmentRenderType::None) || renderType == static_cast<uint32>(EnvironmentRenderType::Opaque) || renderType == static_cast<uint32>(EnvironmentRenderType::InOrderBlend) || renderType == static_cast<uint32>(EnvironmentRenderType::AfterEdge),
			false, "지원하지 않는 환경 오브젝트 렌더 타입입니다. type=%u", renderType);

		outObject.renderType = static_cast<EnvironmentRenderType>(renderType);
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.modelIndex), false, "환경 오브젝트의 모델 인덱스를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outObject.world), false, "환경 오브젝트의 월드 행렬을 읽는 데 실패했습니다.");
		uint32 componentCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, componentCount), false, "환경 오브젝트의 Component 개수를 읽는 데 실패했습니다.");
		outObject.components.reserve(componentCount);
		for (uint32 componentIndex = 0; componentIndex < componentCount; ++componentIndex)
			GM_ASSERT_RETURN_VAL(ReadEnvironmentComponent(inputStream, outObject), false, "환경 오브젝트의 Component를 읽는 데 실패했습니다. index=%u", componentIndex);

		return true;
	}

	bool BinaryMapLoader::ReadEnvironmentComponent(std::istream& inputStream, EnvironmentObjectData& outObject)
	{
		uint32 componentType = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, componentType), false, "환경 Component 타입을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(componentType < static_cast<uint32>(EnvironmentComponentType::Count), false, "지원하지 않는 환경 Component 타입입니다. type=%u", componentType);

		switch (static_cast<EnvironmentComponentType>(componentType))
		{
		case EnvironmentComponentType::BeatMove:
		{
			BeatMoveComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadTriggerBinding(inputStream, data.triggerBindingData), false, "BeatMove 트리거 바인딩을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.targetPosition) && ReadBinary(inputStream, data.durationBeats), false, "BeatMove Component를 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatTriggeredRotation:
		{
			BeatTriggeredRotationComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadTriggerBinding(inputStream, data.triggerBindingData), false, "BeatTriggeredRotation 트리거 바인딩을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.axis) && ReadBinary(inputStream, data.angleDegrees) && ReadBinary(inputStream, data.durationBeats), false, "BeatTriggeredRotation Component를 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatPositionSequence:
		{
			BeatPositionSequenceComponentData data{};
			uint32 positionCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, positionCount), false, "BeatPositionSequence 위치 개수를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryVector(inputStream, data.positionOffsets, positionCount), false, "BeatPositionSequence 위치 목록을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.beatsPerStep) && ReadBinary(inputStream, data.interpolationSpeed), false, "BeatPositionSequence 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatVisibility:
		{
			BeatVisibilityComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadTriggerBinding(inputStream, data.triggerBindingData), false, "BeatVisibility 트리거 바인딩을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.initialVisible) && ReadBinary(inputStream, data.visibleOnTrigger), false, "BeatVisibility 표시 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(data);
			return true;
		}

		case EnvironmentComponentType::BeatTransform:
		{
			BeatTransformComponentData data{};
			uint32 transformType = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, transformType), false, "BeatTransform 타입을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(transformType <= static_cast<uint32>(EnvironmentBeatTransformType::ScaleMultiplier), false, "지원하지 않는 BeatTransform 타입입니다. type=%u", transformType);
			data.type = static_cast<EnvironmentBeatTransformType>(transformType);
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.positionOffset) && ReadBinary(inputStream, data.maxScaleMultiplier), false, "BeatTransform 변형값을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.cycleBeats) && ReadBinary(inputStream, data.phaseOffsetBeats), false, "BeatTransform 비트 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.overrideInitialScale) && ReadBinary(inputStream, data.initialScale), false, "BeatTransform 초기 스케일을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatOrbit:
		{
			BeatOrbitComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.center), false, "BeatOrbit 중심을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.evenBeatDeltaDegrees) && ReadBinary(inputStream, data.oddBeatDeltaDegrees), false, "BeatOrbit 회전값을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.interpolationSpeed) && ReadBinary(inputStream, data.faceCenter), false, "BeatOrbit 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatAudioLevelMove:
		{
			BeatAudioLevelMoveComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.direction), false, "BeatAudioLevelMove 방향을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.maxDistance), false, "BeatAudioLevelMove 최대 이동 거리를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.cycleBeats) && ReadBinary(inputStream, data.phaseOffsetBeats), false, "BeatAudioLevelMove 비트 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatStaticMeshCycle:
		{
			BeatStaticMeshCycleComponentData data{};
			uint32 modelCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, modelCount), false, "BeatStaticMeshCycle 모델 개수를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryVector(inputStream, data.modelIndices, modelCount), false, "BeatStaticMeshCycle 모델 목록을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatSkeletalAnimationSync:
		{
			BeatSkeletalAnimationSyncComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.cycleBeats) && ReadBinary(inputStream, data.phaseOffsetBeats), false, "BeatSkeletalAnimationSync 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BeatTriggeredSkeletalAnimation:
		{
			BeatTriggeredSkeletalAnimationComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadTriggerBinding(inputStream, data.triggerBindingData), false, "BeatTriggeredSkeletalAnimation 트리거 바인딩을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.clipName), false, "BeatTriggeredSkeletalAnimation Clip 이름을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.clipName.empty() == false, false, "BeatTriggeredSkeletalAnimation Clip 이름이 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.initiallyVisible), false, "BeatTriggeredSkeletalAnimation 초기 표시 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.hideWhenCompleted), false, "BeatTriggeredSkeletalAnimation 완료 표시 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.disableCollidersWhenCompleted), false, "BeatTriggeredSkeletalAnimation 완료 Collider 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::BoxCollider3D:
		{
			BoxCollider3DComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "BoxCollider3D의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.localCenter), false, "BoxCollider3D의 Local Center를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.localRotation), false, "BoxCollider3D의 Local Rotation을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.size), false, "BoxCollider3D의 크기를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.collisionFilter.layer) && ReadBinary(inputStream, data.collisionFilter.mask), false, "BoxCollider3D의 충돌 필터를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.isTrigger), false, "BoxCollider3D의 Trigger 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::SphereCollider3D:
		{
			SphereCollider3DComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "SphereCollider3D의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.localCenter), false, "SphereCollider3D의 Local Center를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.radius), false, "SphereCollider3D의 반지름을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.collisionFilter.layer) && ReadBinary(inputStream, data.collisionFilter.mask), false, "SphereCollider3D의 충돌 필터를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.isTrigger), false, "SphereCollider3D의 Trigger 설정을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::Health:
		{
			HealthComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.maxHealth), false, "Health의 Max Health를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.damageInvincibilityDuration), false, "Health의 피격 무적 시간을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::HurtBox:
		{
			HurtBoxComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "HurtBox의 Collider ID를 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::HitReaction:
		{
			HitReactionComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.completionTriggerId), false, "HitReaction의 완료 Trigger ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.completionTriggerId.empty() == false, false, "HitReaction의 완료 Trigger ID가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.reactionAnimationClipName), false, "HitReaction의 반응 Animation Clip 이름을 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::SceneTransitionTrigger:
		{
			SceneTransitionTriggerComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "SceneTransitionTrigger의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "SceneTransitionTrigger의 Collider ID가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.targetSceneName), false, "SceneTransitionTrigger의 Target Scene 이름을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.targetSceneName.empty() == false, false, "SceneTransitionTrigger의 Target Scene 이름이 비어 있습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::CollisionTrigger:
		{
			CollisionTriggerComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "CollisionTrigger의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "CollisionTrigger의 Collider ID가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.triggerId), false, "CollisionTrigger의 Trigger ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.triggerId.empty() == false, false, "CollisionTrigger의 Trigger ID가 비어 있습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::RespawnPointTrigger:
		{
			RespawnPointTriggerComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "RespawnPointTrigger의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "RespawnPointTrigger의 Collider ID가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.respawnPosition), false, "RespawnPointTrigger의 Respawn Position을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.respawnRotationY), false, "RespawnPointTrigger의 Respawn Rotation Y를 읽는 데 실패했습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::MovementBase:
		{
			MovementBaseComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "MovementBase의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "MovementBase의 Collider ID가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.passengerMask), false, "MovementBase의 Passenger Mask를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.passengerMask != 0, false, "MovementBase의 Passenger Mask가 비어 있습니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		case EnvironmentComponentType::FallRespawnTrigger:
		{
			FallRespawnTriggerComponentData data{};
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, data.colliderId), false, "FallRespawnTrigger의 Collider ID를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.colliderId.empty() == false, false, "FallRespawnTrigger의 Collider ID가 비어 있습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, data.damage), false, "FallRespawnTrigger의 Damage를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(data.damage > 0, false, "FallRespawnTrigger의 Damage는 0보다 커야 합니다.");
			outObject.components.emplace_back(std::move(data));
			return true;
		}

		default:
			return false;
		}
	}

	bool BinaryMapLoader::ReadMonsterSpawn(std::istream& inputStream, MonsterSpawnData& outSpawnData)
	{
		uint32 monsterType = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, monsterType), false, "Monster Type을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(monsterType < static_cast<uint32>(MonsterType::Count), false, "지원하지 않는 Monster Type입니다. type=%u", monsterType);

		outSpawnData.type = static_cast<MonsterType>(monsterType);
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.world), false, "Monster World 행렬을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, outSpawnData.activationTriggerId), false, "Monster Activation Trigger ID를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.playAppearanceAnimation), false, "Monster Appearance Animation 설정을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.appearanceBeatOffset), false, "Monster Appearance Beat Offset을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.combatBeatOffset), false, "Monster Combat Beat Offset을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.maxHealth), false, "Monster Max Health를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.isInvincible), false, "Monster 무적 설정을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.bodyColliderCenter), false, "Monster Body Collider Center를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.bodyColliderSize), false, "Monster Body Collider Size를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.moveSpeed), false, "Monster Move Speed를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.rotationInterpSpeed), false, "Monster Rotation Interp Speed를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.attackCooldownBeats), false, "Monster Attack Cooldown을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.attackDamage), false, "Monster Attack Damage를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.attackRangeMin), false, "Monster Attack Range Min을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outSpawnData.attackRangeMax), false, "Monster Attack Range Max를 읽는 데 실패했습니다.");
		return true;
	}

	bool BinaryMapLoader::ReadTriggerBinding(std::istream& inputStream, TriggerBindingData& outBinding)
	{
		GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, outBinding.triggerId), false, "Trigger ID를 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(outBinding.triggerId.empty() == false, false, "Trigger ID가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outBinding.beatOffset), false, "트리거 Beat Offset을 읽는 데 실패했습니다.");
		GM_ASSERT_RETURN_VAL(std::isfinite(outBinding.beatOffset) && outBinding.beatOffset >= 0.f, false, "트리거 Beat Offset이 유효하지 않습니다.");
		return true;
	}

	bool BinaryMapLoader::Load(const std::wstring& filePath, MapData& outMapData)
	{
		std::ifstream inputStream(std::filesystem::path(filePath), std::ios::binary);
		GM_ASSERT_RETURN_VAL(inputStream.is_open(), false, "맵 바이너리 파일을 열지 못했습니다. path=%ls", filePath.c_str());

		uint32 objectCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, objectCount), false, "환경 오브젝트 개수를 읽는 데 실패했습니다.");

		MapData loadedData{};
		loadedData.objects.resize(objectCount);
		for (uint32 objectIndex = 0; objectIndex < objectCount; ++objectIndex)
		{
			EnvironmentObjectData& object = loadedData.objects[objectIndex];
			GM_ASSERT_RETURN_VAL(ReadEnvironmentObject(inputStream, object), false, "환경 오브젝트 데이터를 읽는 데 실패했습니다. index=%u", objectIndex);
		}

		if (inputStream.peek() != std::char_traits<char>::eof())
		{
			uint32 monsterSpawnCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, monsterSpawnCount), false, "Monster Spawn 개수를 읽는 데 실패했습니다.");

			loadedData.monsterSpawnDatas.resize(monsterSpawnCount);
			for (uint32 spawnIndex = 0; spawnIndex < monsterSpawnCount; ++spawnIndex)
			{
				MonsterSpawnData& spawnData = loadedData.monsterSpawnDatas[spawnIndex];
				GM_ASSERT_RETURN_VAL(ReadMonsterSpawn(inputStream, spawnData), false, "Monster Spawn 데이터를 읽는 데 실패했습니다. index=%u", spawnIndex);
			}
		}

		GM_ASSERT_RETURN_VAL(inputStream.peek() == std::char_traits<char>::eof(), false, "맵 바이너리 끝에 해석되지 않은 데이터가 남아 있습니다. path=%ls", filePath.c_str());
		outMapData = std::move(loadedData);
		return true;
	}
}
