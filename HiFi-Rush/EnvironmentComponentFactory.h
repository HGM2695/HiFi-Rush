#pragma once

#include "EnvironmentComponentTypes.h"

namespace gm
{
	class BeatSystem;
	class GameObject;
	class Resources;

	class EnvironmentComponentFactory
	{
	public:
		EnvironmentComponentFactory(Resources& resources, const BeatSystem& beatSystem);

		bool AddComponents(GameObject& gameObject, const std::vector<EnvironmentComponentData>& components) const;

	private:
		bool CreateComponent(GameObject& gameObject, const BeatMoveComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTriggeredRotationComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTriggeredRotationShakeComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatPositionSequenceComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatVisibilityComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTransformComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatOrbitComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatAudioLevelMoveComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatStaticMeshCycleComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTextureSequenceComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const TriggeredMaterialOverrideComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatSkeletalAnimationSyncComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTriggeredSkeletalAnimationComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BoxCollider3DComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const SphereCollider3DComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const HealthComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const HurtBoxComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const HitReactionComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const SceneTransitionTriggerComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const CollisionTriggerComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const RespawnPointTriggerComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const MovementBaseComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const FallRespawnTriggerComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const DirectionalLightComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const PointLightComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const SpotLightComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const ContinuousRotationComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const TriggeredLightColorComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTextureUVScrollComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatTextureUVStepComponentData& data) const;
		bool CreateComponent(GameObject& gameObject, const BeatMaterialBrightnessPulseComponentData& data) const;

	private:
		Resources&			_resources;
		const BeatSystem&	_beatSystem;
	};
}
