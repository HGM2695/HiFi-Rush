#include "EffectPresets.h"

#include "BinaryIO.h"
#include "GMAssert.h"

#include <fstream>
#include <utility>

namespace gm
{
	namespace
	{
		bool ReadEffectTime(std::istream& inputStream, EffectTime& outTime)
		{
			uint32 unit = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTime.value) && ReadBinary(inputStream, unit), false, "Effect Time을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(unit < static_cast<uint32>(EffectTimeUnit::Count), false, "Effect Time Unit이 유효하지 않습니다.");
			outTime.unit = static_cast<EffectTimeUnit>(unit);
			return true;
		}

		bool ReadEffectTrack(std::istream& inputStream, EffectTrackData& outTrack)
		{
			uint32 visualType = 0;
			uint32 attachmentMode = 0;
			uint32 facingMode = 0;
			uint32 animationType = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, visualType) && ReadBinary(inputStream, attachmentMode), false, "Effect Visual Type과 Attachment Mode를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(visualType < static_cast<uint32>(EffectVisualType::Count), false, "Effect Visual Type이 유효하지 않습니다.");
			GM_ASSERT_RETURN_VAL(attachmentMode < static_cast<uint32>(EffectTrackAttachmentMode::Count), false, "Effect Track Attachment Mode가 유효하지 않습니다.");
			outTrack.visualType = static_cast<EffectVisualType>(visualType);
			outTrack.attachmentMode = static_cast<EffectTrackAttachmentMode>(attachmentMode);
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, outTrack.resourceKey) && ReadBinaryWideString(inputStream, outTrack.baseColorTextureKey) && ReadBinaryWideString(inputStream, outTrack.attachmentSocketName), false, "Effect Resource Key와 Attachment Socket Name을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.localTransform), false, "Effect Local Transform을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.startScale) && ReadBinary(inputStream, outTrack.endScale) && ReadBinary(inputStream, outTrack.endPositionOffset) && ReadBinary(inputStream, outTrack.endRotationOffsetDegrees), false, "Effect Transform Animation을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.positionInterpolationSpeed) && ReadBinary(inputStream, outTrack.rotationInterpolationSpeed) && ReadBinary(inputStream, outTrack.scaleInterpolationSpeed) && ReadEffectTime(inputStream, outTrack.scaleInterpolationDelay), false, "Effect Transform Interpolation 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.startAngularVelocityDegrees) && ReadBinary(inputStream, outTrack.endAngularVelocityDegrees) && ReadBinary(inputStream, outTrack.angularVelocityInterpolationSpeed), false, "Effect Angular Velocity Animation을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.randomPositionMinimum) && ReadBinary(inputStream, outTrack.randomPositionMaximum) && ReadBinary(inputStream, outTrack.randomRotationMinimumDegrees) && ReadBinary(inputStream, outTrack.randomRotationMaximumDegrees) && ReadBinary(inputStream, outTrack.randomEndPositionOffsetMinimum) && ReadBinary(inputStream, outTrack.randomEndPositionOffsetMaximum) && ReadBinary(inputStream, outTrack.randomEndScaleMinimum) && ReadBinary(inputStream, outTrack.randomEndScaleMaximum), false, "Effect Random Transform 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, animationType) && ReadBinary(inputStream, outTrack.animationSpeed), false, "Effect Animation 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(animationType < static_cast<uint32>(EffectAnimationType::Count), false, "Effect Animation Type이 유효하지 않습니다.");
			outTrack.animationType = static_cast<EffectAnimationType>(animationType);
			GM_ASSERT_RETURN_VAL(ReadEffectTime(inputStream, outTrack.delay) && ReadEffectTime(inputStream, outTrack.lifetime), false, "Effect 시간 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, facingMode), false, "Effect Sprite Facing Mode를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(facingMode < static_cast<uint32>(SpriteFacingMode::Count), false, "Effect Sprite Facing Mode가 유효하지 않습니다.");
			outTrack.facingMode = static_cast<SpriteFacingMode>(facingMode);
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.sortDepthOffset), false, "Effect Sort Depth Offset을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.spriteSheetColumns) && ReadBinary(inputStream, outTrack.spriteSheetRows) && ReadBinary(inputStream, outTrack.spriteSheetStartFrame) && ReadBinary(inputStream, outTrack.spriteSheetFrameCount) && ReadEffectTime(inputStream, outTrack.spriteSheetFrameDuration), false, "Effect Sprite Sheet 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.opacity) && ReadBinary(inputStream, outTrack.endOpacity) && ReadBinary(inputStream, outTrack.opacityInterpolationSpeed), false, "Effect Opacity를 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.startFillRatio) && ReadBinary(inputStream, outTrack.endFillRatio) && ReadBinary(inputStream, outTrack.radialCenter) && ReadBinary(inputStream, outTrack.radialStartAngle) && ReadBinary(inputStream, outTrack.radialSweepAngle), false, "Effect Fill 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, outTrack.emissiveColor) && ReadBinary(inputStream, outTrack.emissiveIntensity) && ReadEffectTime(inputStream, outTrack.emissiveDuration), false, "Effect Emissive 설정을 읽는 데 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, outTrack.dissolveTextureKey) && ReadBinary(inputStream, outTrack.dissolveStartThreshold) && ReadBinary(inputStream, outTrack.dissolveEndThreshold), false, "Effect Dissolve 설정을 읽는 데 실패했습니다.");
			return true;
		}
	}

	bool EffectPresets::Load(const std::wstring& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		GM_ASSERT_RETURN_VAL(inputStream.is_open(), false, "Effect Preset 파일을 열 수 없습니다.");

		uint32 presetCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, presetCount), false, "Effect Preset 개수를 읽는 데 실패했습니다.");
		std::vector<EffectPresetData> presets(presetCount);
		for (EffectPresetData& preset : presets)
		{
			GM_ASSERT_RETURN_VAL(ReadBinaryWideString(inputStream, preset.id), false, "Effect Preset ID를 읽는 데 실패했습니다.");
			uint32 trackCount = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, trackCount), false, "Effect Track 개수를 읽는 데 실패했습니다.");
			preset.tracks.resize(trackCount);
			for (EffectTrackData& track : preset.tracks)
				GM_ASSERT_RETURN_VAL(ReadEffectTrack(inputStream, track), false, "Effect Track을 읽는 데 실패했습니다. effect=%ls", preset.id.c_str());
		}

		_presets = std::move(presets);
		return true;
	}

	const EffectPresetData* EffectPresets::Find(const std::wstring& effectId) const
	{
		for (const EffectPresetData& preset : _presets)
		{
			if (preset.id == effectId)
				return &preset;
		}
		return nullptr;
	}

#if GM_ENABLE_DEBUG_TOOLS
	std::optional<float> EffectPresets::FindEmissiveIntensity(const std::wstring& resourceKey) const
	{
		for (const EffectPresetData& preset : _presets)
		{
			for (const EffectTrackData& track : preset.tracks)
			{
				if (track.resourceKey == resourceKey)
					return track.emissiveIntensity;
			}
		}
		return std::nullopt;
	}

	std::optional<float> EffectPresets::FindEmissiveIntensity(const std::wstring& effectId, const std::wstring& resourceKey) const
	{
		const EffectPresetData* preset = Find(effectId);
		if (preset == nullptr)
			return std::nullopt;
		for (const EffectTrackData& track : preset->tracks)
		{
			if (track.resourceKey == resourceKey)
				return track.emissiveIntensity;
		}
		return std::nullopt;
	}

	bool EffectPresets::SetEmissiveIntensity(const std::wstring& resourceKey, float intensity)
	{
		GM_ASSERT_RETURN_VAL(intensity >= 0.f, false, "Effect Emissive Intensity는 0 이상이어야 합니다.");
		bool hasChanged = false;
		for (EffectPresetData& preset : _presets)
		{
			for (EffectTrackData& track : preset.tracks)
			{
				if (track.resourceKey != resourceKey)
					continue;
				track.emissiveIntensity = intensity;
				hasChanged = true;
			}
		}
		return hasChanged;
	}

	bool EffectPresets::SetEmissiveIntensity(const std::wstring& effectId, const std::wstring& resourceKey, float intensity)
	{
		GM_ASSERT_RETURN_VAL(intensity >= 0.f, false, "Effect Emissive Intensity는 0 이상이어야 합니다.");
		for (EffectPresetData& preset : _presets)
		{
			if (preset.id != effectId)
				continue;
			bool hasChanged = false;
			for (EffectTrackData& track : preset.tracks)
			{
				if (track.resourceKey != resourceKey)
					continue;
				track.emissiveIntensity = intensity;
				hasChanged = true;
			}
			return hasChanged;
		}
		return false;
	}
#endif
}
