#pragma once

#include "MathTypes.h"
#include "RenderTypes.h"

#include <string>
#include <vector>

namespace gm
{
	enum class EffectTimeUnit
	{
		Second,
		Beat,

		Count,
	};

	struct EffectTime
	{
		float			value = 0.f;
		EffectTimeUnit	unit = EffectTimeUnit::Second;

		static EffectTime CreateSecond(float second) { return { second, EffectTimeUnit::Second }; }
		static EffectTime CreateBeat(float beat) { return { beat, EffectTimeUnit::Beat }; }
		bool HasElapsed(float elapsedSecond, float elapsedBeat) const { return unit == EffectTimeUnit::Second ? elapsedSecond >= value : elapsedBeat >= value; }
	};

	enum class EffectVisualType
	{
		Sprite,
		StaticMesh,

		Count,
	};

	enum class EffectTrackAttachmentMode
	{
		Inherit,
		WorldAtSpawn,
		FollowOwnerPosition,
		FollowSocketPosition,

		Count,
	};

	enum class EffectAnimationType
	{
		Interpolation,
		RotationCircle,
		BeatAcceleratedInterpolation,
		ContinuousRotationCircle,

		Count,
	};

	struct EffectTrackData
	{
		EffectVisualType	visualType = EffectVisualType::Sprite;
		EffectTrackAttachmentMode attachmentMode = EffectTrackAttachmentMode::Inherit;
		std::wstring		resourceKey{};
		std::wstring		baseColorTextureKey{};
		std::wstring		attachmentSocketName{};
		Matrix				localTransform = Matrix::Identity;
		Vector3				startScale{ 1.f, 1.f, 1.f };
		Vector3				endScale{ 1.f, 1.f, 1.f };
		Vector3				endPositionOffset{};
		Vector3				endRotationOffsetDegrees{};
		float				positionInterpolationSpeed = 0.f;
		float				rotationInterpolationSpeed = 0.f;
		float				scaleInterpolationSpeed = 0.f;
		EffectTime			scaleInterpolationDelay{};
		Vector3				startAngularVelocityDegrees{};
		Vector3				endAngularVelocityDegrees{};
		float				angularVelocityInterpolationSpeed = 0.f;
		Vector3				randomPositionMinimum{};
		Vector3				randomPositionMaximum{};
		Vector3				randomRotationMinimumDegrees{};
		Vector3				randomRotationMaximumDegrees{};
		Vector3				randomEndPositionOffsetMinimum{};
		Vector3				randomEndPositionOffsetMaximum{};
		Vector3				randomEndScaleMinimum{};
		Vector3				randomEndScaleMaximum{};
		EffectAnimationType animationType = EffectAnimationType::Interpolation;
		float				animationSpeed = 1.f;
		EffectTime			delay{};
		EffectTime			lifetime = EffectTime::CreateSecond(1.f);
		SpriteFacingMode	facingMode = SpriteFacingMode::None;
		float				sortDepthOffset = 0.f;
		uint32				spriteSheetColumns = 1;
		uint32				spriteSheetRows = 1;
		uint32				spriteSheetStartFrame = 0;
		uint32				spriteSheetFrameCount = 1;
		EffectTime			spriteSheetFrameDuration{};
		float				opacity = 1.f;
		float				endOpacity = 1.f;
		float				opacityInterpolationSpeed = 0.f;
		float				startFillRatio = 1.f;
		float				endFillRatio = 1.f;
		Vector2				radialCenter{ 0.5f, 0.5f };
		float				radialStartAngle = 0.f;
		float				radialSweepAngle = 0.f;
		Color				emissiveColor = Colors::White;
		float				emissiveIntensity = 0.f;
		EffectTime			emissiveDuration{};
		std::wstring		dissolveTextureKey{};
		float				dissolveStartThreshold = 0.f;
		float				dissolveEndThreshold = 0.f;
	};

	struct EffectPresetData
	{
		std::wstring					id{};
		std::vector<EffectTrackData>	tracks{};
	};
}
