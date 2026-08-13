#pragma once

#include "CollisionTypes.h"
#include "MathTypes.h"
#include "Types.h"

#include <string>
#include <variant>
#include <vector>

namespace gm
{
	struct TriggerSequenceBindingData
	{
		std::wstring	sequenceId{};
		float			beatOffset = 0.f;
	};

	enum class EnvironmentBeatTransformType : uint32
	{
		PositionOffset,
		ScaleMultiplier,
	};

	struct BeatMoveComponentData
	{
		TriggerSequenceBindingData	triggerBindingData{};
		Vector3						targetPosition{};
		float						durationBeats = 1.f;
	};

	struct BeatTriggeredRotationComponentData
	{
		TriggerSequenceBindingData	triggerBindingData{};
		Vector3						axis{ 0.f, 0.f, 1.f };
		float						angleDegrees = 90.f;
		float						durationBeats = 0.25f;
	};

	struct BeatPositionSequenceComponentData
	{
		std::vector<Vector3>	positionOffsets{};
		float					beatsPerStep = 1.f;
		float					interpolationSpeed = 6.3f;
	};

	struct BeatVisibilityComponentData
	{
		TriggerSequenceBindingData	triggerBindingData{};
		bool						initialVisible = false;
		bool						visibleOnTrigger = true;
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

	struct BeatSkeletalAnimationSyncComponentData
	{
		float cycleBeats = 1.f;
		float phaseOffsetBeats = 0.f;
	};

	struct BeatTriggeredSkeletalAnimationComponentData
	{
		TriggerSequenceBindingData	triggerBindingData{};
		std::wstring				clipName = L"Default";
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
		std::wstring	completionSequenceId{};
		std::wstring	reactionAnimationClipName{};
	};

	struct SceneTransitionTriggerComponentData
	{
		std::wstring	colliderId{};
		std::wstring	targetSceneName{};
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
		SceneTransitionTriggerComponentData>;

	static_assert(std::variant_size_v<EnvironmentComponentData> == static_cast<size_t>(EnvironmentComponentType::Count));
}
