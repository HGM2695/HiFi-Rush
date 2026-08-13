#pragma once

#include "EnvironmentComponentTypes.h"

namespace gm
{
	class BeatSystem;
	class GameObject;
	class IBeatTriggerAction;
	class Resources;

	struct EnvironmentTriggerAction
	{
		TriggerSequenceBindingData	triggerBindingData{};
		IBeatTriggerAction*			actionComponent = nullptr;
	};

	class EnvironmentComponentFactory
	{
	public:
		EnvironmentComponentFactory(Resources& resources, const BeatSystem& beatSystem);

		bool AddComponents(
			GameObject& gameObject,
			const std::vector<EnvironmentComponentData>& components,
			std::vector<EnvironmentTriggerAction>& outTriggerActions) const;

	private:
		bool CreateComponent(GameObject& gameObject, const BeatMoveComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatTriggeredRotationComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatPositionSequenceComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatVisibilityComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatTransformComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatOrbitComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatAudioLevelMoveComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatStaticMeshCycleComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatSkeletalAnimationSyncComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BeatTriggeredSkeletalAnimationComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const BoxCollider3DComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const SphereCollider3DComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const HealthComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const HurtBoxComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;
		bool CreateComponent(GameObject& gameObject, const HitTriggeredEnvironmentComponentData& data, std::vector<EnvironmentTriggerAction>& outTriggerActions) const;

	private:
		Resources&			_resources;
		const BeatSystem&	_beatSystem;
	};
}
