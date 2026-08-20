#pragma once

#include "CollisionTypes.h"
#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "Types.h"

#include <string>
#include <variant>
#include <vector>

namespace gm
{
	struct TriggerBindingData
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
	};

	enum class EnvironmentBeatTransformType : uint32
	{
		PositionOffset,
		ScaleMultiplier,
	};

	struct BeatMoveComponentData
	{
		TriggerBindingData		triggerBindingData{};
		Vector3					targetPosition{};
		float					durationBeats = 1.f;
	};

	struct BeatTriggeredRotationComponentData
	{
		TriggerBindingData		triggerBindingData{};
		Vector3					axis{ 0.f, 0.f, 1.f };
		float					angleDegrees = 90.f;
		float					durationBeats = 0.25f;
	};

	struct BeatTriggeredRotationShakeComponentData
	{
		TriggerBindingData	triggerBindingData{};
		Vector3				axis{ 1.f, 0.f, 0.f };
		float				angleDegrees = 15.f;
		float				durationBeats = 0.75f;
	};

	struct BeatPositionSequenceComponentData
	{
		std::vector<Vector3>	positionOffsets{};
		float					beatsPerStep = 1.f;
		float					interpolationSpeed = 6.3f;
	};

	struct BeatVisibilityComponentData
	{
		TriggerBindingData		triggerBindingData{};
		bool					initialVisible = false;
		bool					visibleOnTrigger = true;
	};

	struct BeatTransformComponentData
	{
		EnvironmentBeatTransformType	type = EnvironmentBeatTransformType::PositionOffset;
		Vector3							positionOffset{};
		Vector3							maxScaleMultiplier{ 1.f, 1.f, 1.f };
		float							cycleBeats = 1.f;
		float							phaseOffsetBeats = 0.f;
		bool							overrideInitialScale = false;
		Vector3							initialScale{ 1.f, 1.f, 1.f };
	};

	struct BeatOrbitComponentData
	{
		Vector3	center{};
		float	evenBeatDeltaDegrees = -10.f;
		float	oddBeatDeltaDegrees = -5.f;
		float	interpolationSpeed = 13.f;
		bool	faceCenter = false;
	};

	struct BeatAudioLevelMoveComponentData
	{
		Vector3	direction{ 0.f, 1.f, 0.f };
		float	maxDistance = 1.f;
		float	cycleBeats = 1.f;
		float	phaseOffsetBeats = 0.f;
	};

	struct BeatStaticMeshCycleComponentData
	{
		std::vector<uint32> modelIndices{};
	};

	struct BeatTextureSequenceComponentData
	{
		TriggerBindingData			triggerBindingData{};
		uint32						materialSlot = 0;
		TextureSlot					textureSlot = TextureSlot::BaseColor;
		std::vector<std::wstring>	initialTextureKeys{};
		std::vector<std::wstring>	triggeredTextureKeys{};
		float						framesPerBeat = 1.f;
		float						phaseOffsetBeats = 0.f;
	};

	struct MaterialTextureOverrideData
	{
		uint32			materialSlot = 0;
		TextureSlot		textureSlot = TextureSlot::BaseColor;
		std::wstring	textureKey{};
	};

	struct TriggeredMaterialOverrideComponentData
	{
		TriggerBindingData					triggerBindingData{};
		std::vector<MaterialTextureOverrideData>	overrides{};
	};

	struct BeatSkeletalAnimationSyncComponentData
	{
		float cycleBeats = 1.f;
		float phaseOffsetBeats = 0.f;
	};

	struct BeatTriggeredSkeletalAnimationComponentData
	{
		TriggerBindingData	triggerBindingData{};
		std::wstring			clipName = L"Default";
		bool				initiallyVisible = false;
		bool				hideWhenCompleted = true;
		bool				disableCollidersWhenCompleted = false;
	};

	struct BoxCollider3DComponentData
	{
		std::wstring	colliderId{};
		Vector3			localCenter{};
		Quaternion		localRotation{ 0.f, 0.f, 0.f, 1.f };
		Vector3			size{ 1.f, 1.f, 1.f };
		CollisionFilter	collisionFilter{};
		bool			isTrigger = false;
	};

	struct SphereCollider3DComponentData
	{
		std::wstring	colliderId{};
		Vector3			localCenter{};
		float			radius = 0.5f;
		CollisionFilter	collisionFilter{};
		bool			isTrigger = false;
	};

	struct HealthComponentData
	{
		int32	maxHealth = 1;
		float	damageInvincibilityDuration = 0.f;
	};

	struct HurtBoxComponentData
	{
		std::wstring colliderId{};
	};

	struct HitReactionComponentData
	{
		std::wstring	completionTriggerId{};
		std::wstring	reactionAnimationClipName{};
	};

	struct SceneTransitionTriggerComponentData
	{
		std::wstring	colliderId{};
		std::wstring	targetSceneName{};
	};

	struct CollisionTriggerComponentData
	{
		std::wstring	colliderId{};
		std::wstring	triggerId{};
	};

	struct RespawnPointTriggerComponentData
	{
		std::wstring	colliderId{};
		Vector3			respawnPosition{};
		float			respawnRotationY = 0.f;
	};

	struct MovementBaseComponentData
	{
		std::wstring	colliderId{};
		CollisionMask	passengerMask = AllCollisionLayers;
	};

	struct FallRespawnTriggerComponentData
	{
		std::wstring	colliderId{};
		int32			damage = 50;
	};

	enum class EnvironmentComponentType : uint32
	{
		BeatMove,
		BeatTriggeredRotation,
		BeatPositionSequence,
		BeatVisibility,
		BeatTransform,
		BeatOrbit,
		BeatAudioLevelMove,
		BeatStaticMeshCycle,
		BeatSkeletalAnimationSync,
		BeatTriggeredSkeletalAnimation,
		BoxCollider3D,
		SphereCollider3D,
		Health,
		HurtBox,
		HitReaction,
		SceneTransitionTrigger,
		CollisionTrigger,
		RespawnPointTrigger,
		MovementBase,
		FallRespawnTrigger,
		BeatTriggeredRotationShake,
		BeatTextureSequence,
		TriggeredMaterialOverride,
		Count,
	};

	using EnvironmentComponentData = std::variant<
		BeatMoveComponentData,
		BeatTriggeredRotationComponentData,
		BeatPositionSequenceComponentData,
		BeatVisibilityComponentData,
		BeatTransformComponentData,
		BeatOrbitComponentData,
		BeatAudioLevelMoveComponentData,
		BeatStaticMeshCycleComponentData,
		BeatSkeletalAnimationSyncComponentData,
		BeatTriggeredSkeletalAnimationComponentData,
		BoxCollider3DComponentData,
		SphereCollider3DComponentData,
		HealthComponentData,
		HurtBoxComponentData,
		HitReactionComponentData,
		SceneTransitionTriggerComponentData,
		CollisionTriggerComponentData,
		RespawnPointTriggerComponentData,
		MovementBaseComponentData,
		FallRespawnTriggerComponentData,
		BeatTriggeredRotationShakeComponentData,
		BeatTextureSequenceComponentData,
		TriggeredMaterialOverrideComponentData>;

	static_assert(std::variant_size_v<EnvironmentComponentData> == static_cast<size_t>(EnvironmentComponentType::Count));
}
