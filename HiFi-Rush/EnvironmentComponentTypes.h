#pragma once

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
		BeatTriggeredSkeletalAnimationComponentData>;

	static_assert(std::variant_size_v<EnvironmentComponentData> == static_cast<size_t>(EnvironmentComponentType::Count));
}
