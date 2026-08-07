#include "EnvironmentComponentFactory.h"
#include "BeatAudioLevelMoveComponent.h"
#include "BeatMoveComponent.h"
#include "BeatOrbitComponent.h"
#include "BeatPositionSequenceComponent.h"
#include "BeatSkeletalAnimationSyncComponent.h"
#include "BeatStaticMeshCycleComponent.h"
#include "BeatTransformComponent.h"
#include "BeatTriggeredRotationComponent.h"
#include "BeatTriggeredSkeletalAnimationComponent.h"
#include "BeatVisibilityComponent.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "IBeatTriggerAction.h"
#include "Resources.h"
#include "SkeletalAnimatorComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"

#include <string>

namespace gm
{
	EnvironmentComponentFactory::EnvironmentComponentFactory(Resources& resources, const BeatSystem& beatSystem)
		: _resources(resources), _beatSystem(beatSystem)
	{}

	bool EnvironmentComponentFactory::AddComponents(GameObject& gameObject, const std::vector<EnvironmentComponentData>& components, std::vector<EnvironmentTriggerAction>& outTriggerActions) const
	{
		outTriggerActions.clear();
		for (const EnvironmentComponentData& component : components)
		{
			const bool succeeded = std::visit(
				[this, &gameObject, &outTriggerActions](const auto& data)
				{
					return CreateComponent(gameObject, data, outTriggerActions);
				}, component);

			if (succeeded == false)
				return false;
		}

		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatMoveComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const
	{
		BeatMoveDesc desc{};
		desc.targetPosition = data.targetPosition;
		desc.durationBeats = data.durationBeats;
		BeatMoveComponent* component = gameObject.AddComponent<BeatMoveComponent>(_beatSystem, desc);
		GM_ASSERT_RETURN_VAL(component, false, "BeatMoveComponent 생성에 실패했습니다.");
		outTriggerActions.push_back(EnvironmentTriggerAction{ data.triggerBindingData, component });
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTriggeredRotationComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const
	{
		BeatTriggeredRotationDesc desc{};
		desc.axis = data.axis;
		desc.angleDegrees = data.angleDegrees;
		desc.durationBeats = data.durationBeats;
		BeatTriggeredRotationComponent* component = gameObject.AddComponent<BeatTriggeredRotationComponent>(_beatSystem, desc);
		GM_ASSERT_RETURN_VAL(component, false, "BeatTriggeredRotationComponent 생성에 실패했습니다.");
		outTriggerActions.push_back(EnvironmentTriggerAction{ data.triggerBindingData, component });
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatPositionSequenceComponentData& data, std::vector<EnvironmentTriggerAction>&) const
	{
		BeatPositionSequenceDesc desc{};
		desc.positionOffsets = data.positionOffsets;
		desc.beatsPerStep = data.beatsPerStep;
		desc.interpolationSpeed = data.interpolationSpeed;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatPositionSequenceComponent>(_beatSystem, desc), false, "BeatPositionSequenceComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatVisibilityComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const
	{
		BeatVisibilityDesc desc{};
		desc.initialVisible = data.initialVisible;
		desc.visibleOnTrigger = data.visibleOnTrigger;
		BeatVisibilityComponent* component = gameObject.AddComponent<BeatVisibilityComponent>(_beatSystem, desc);
		GM_ASSERT_RETURN_VAL(component, false, "BeatVisibilityComponent 생성에 실패했습니다.");
		outTriggerActions.push_back(EnvironmentTriggerAction{ data.triggerBindingData, component });
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTransformComponentData& data, std::vector<EnvironmentTriggerAction>&) const
	{
		TransformComponent* transform = gameObject.GetTransform();
		GM_ASSERT_RETURN_VAL(transform, false, "BeatTransformComponent에 필요한 TransformComponent가 없습니다.");
		if (data.overrideInitialScale)
			transform->SetScale(data.initialScale);

		BeatTransformDesc desc{};
		desc.type = data.type == EnvironmentBeatTransformType::PositionOffset ? BeatTransformType::PositionOffset : BeatTransformType::ScaleMultiplier;
		desc.positionOffset = data.positionOffset;
		desc.maxScaleMultiplier = data.maxScaleMultiplier;
		desc.cycleBeats = data.cycleBeats;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatTransformComponent>(_beatSystem, desc), false, "BeatTransformComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatOrbitComponentData& data, std::vector<EnvironmentTriggerAction>&) const
	{
		BeatOrbitDesc desc{};
		desc.center = data.center;
		desc.evenBeatDeltaDegrees = data.evenBeatDeltaDegrees;
		desc.oddBeatDeltaDegrees = data.oddBeatDeltaDegrees;
		desc.interpolationSpeed = data.interpolationSpeed;
		desc.faceCenter = data.faceCenter;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatOrbitComponent>(_beatSystem, desc), false, "BeatOrbitComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatAudioLevelMoveComponentData& data, std::vector<EnvironmentTriggerAction>&) const
	{
		BeatAudioLevelMoveDesc desc{};
		desc.direction = data.direction;
		desc.maxDistance = data.maxDistance;
		desc.cycleBeats = data.cycleBeats;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatAudioLevelMoveComponent>(_beatSystem, desc), false, "BeatAudioLevelMoveComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatStaticMeshCycleComponentData& data, std::vector<EnvironmentTriggerAction>&) const
	{
		StaticMeshComponent* meshComponent = gameObject.GetComponent<StaticMeshComponent>();
		GM_ASSERT_RETURN_VAL(meshComponent, false, "BeatStaticMeshCycleComponent에 필요한 StaticMeshComponent가 없습니다.");
		GM_ASSERT_RETURN_VAL(data.modelIndices.empty() == false, false, "StaticMesh 전환에 사용할 모델 목록이 비어 있습니다.");

		std::vector<std::shared_ptr<StaticMesh>> meshVariants;
		meshVariants.reserve(data.modelIndices.size());
		for (uint32 modelIndex : data.modelIndices)
		{
			const std::wstring modelKey = L"Environment" + std::to_wstring(modelIndex);
			std::shared_ptr<StaticMesh> variant = _resources.Find<StaticMesh>(modelKey);
			GM_ASSERT_RETURN_VAL(variant, false, "StaticMesh 변형 리소스가 없습니다. key=%ls", modelKey.c_str());
			meshVariants.push_back(std::move(variant));
		}

		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatStaticMeshCycleComponent>(_beatSystem, *meshComponent, std::move(meshVariants)), false, "BeatStaticMeshCycleComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatSkeletalAnimationSyncComponentData& data, std::vector<EnvironmentTriggerAction>&) const
	{
		SkeletalAnimatorComponent* animator = gameObject.GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "BeatSkeletalAnimationSyncComponent에 필요한 SkeletalAnimatorComponent가 없습니다.");

		BeatSkeletalAnimationSyncDesc desc{};
		desc.cycleBeats = data.cycleBeats;
		desc.phaseOffsetBeats = data.phaseOffsetBeats;
		GM_ASSERT_RETURN_VAL(gameObject.AddComponent<BeatSkeletalAnimationSyncComponent>(_beatSystem, *animator, desc), false, "BeatSkeletalAnimationSyncComponent 생성에 실패했습니다.");
		return true;
	}

	bool EnvironmentComponentFactory::CreateComponent(GameObject& gameObject, const BeatTriggeredSkeletalAnimationComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const
	{
		SkeletalAnimatorComponent* animator = gameObject.GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "BeatTriggeredSkeletalAnimationComponent에 필요한 SkeletalAnimatorComponent가 없습니다.");

		BeatTriggeredSkeletalAnimationDesc desc{};
		desc.clipName = data.clipName;
		BeatTriggeredSkeletalAnimationComponent* component = gameObject.AddComponent<BeatTriggeredSkeletalAnimationComponent>(_beatSystem, *animator, std::move(desc));
		GM_ASSERT_RETURN_VAL(component, false, "BeatTriggeredSkeletalAnimationComponent 생성에 실패했습니다.");
		outTriggerActions.push_back(EnvironmentTriggerAction{ data.triggerBindingData, component });
		return true;
	}
}
