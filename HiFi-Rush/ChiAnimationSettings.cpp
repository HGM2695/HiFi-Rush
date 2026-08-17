#include "ChiAnimationSettings.h"
#include "BinaryIO.h"
#include "GMAssert.h"

#include <fstream>

namespace gm
{
	bool ChiAnimationSettings::Load(const std::wstring& filePath)
	{
		std::ifstream inputStream(filePath, std::ios::binary);
		GM_ASSERT_RETURN_VAL(inputStream.is_open(), false, "Chi Animation Settings 파일을 열 수 없습니다.");

		uint32 settingsCount = 0;
		GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, settingsCount), false, "Chi Animation Settings 개수 읽기에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(settingsCount == ChiAnimationClipIdCount, false, "Chi Animation Settings 개수가 ChiAnimationClipId와 일치하지 않습니다.");

		std::array<ChiAnimationSetting, ChiAnimationClipIdCount> settings{};
		for (ChiAnimationSetting& setting : settings)
		{
			uint8 lockInputMovement = 0;
			uint8 useGravity = 0;
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, lockInputMovement), false, "Chi Animation 이동 잠금 설정 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, useGravity), false, "Chi Animation 중력 설정 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.rootMotionWeight.x), false, "Chi Animation Root Motion X 가중치 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.rootMotionWeight.y), false, "Chi Animation Root Motion Y 가중치 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.rootMotionWeight.z), false, "Chi Animation Root Motion Z 가중치 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.blendDuration), false, "Chi Animation Blend 시간 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.playbackStartBeats), false, "Chi Animation 시작 Beat 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.impactMarkerBeat), false, "Chi Animation Impact Marker Beat 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.maxPlaybackRateScale), false, "Chi Animation 최대 재생 속도 배율 읽기에 실패했습니다.");
			GM_ASSERT_RETURN_VAL(ReadBinary(inputStream, setting.actionCancelStartBeats), false, "Chi Animation Action Cancel 시작 Beat 읽기에 실패했습니다.");
			setting.lockInputMovement = lockInputMovement != 0;
			setting.useGravity = useGravity != 0;
			GM_ASSERT_RETURN_VAL(setting.blendDuration >= 0.f, false, "Chi Animation Blend 시간은 0 이상이어야 합니다.");
			GM_ASSERT_RETURN_VAL(setting.maxPlaybackRateScale >= 1.f, false, "Chi Animation 최대 재생 속도 배율은 1 이상이어야 합니다.");
			GM_ASSERT_RETURN_VAL(setting.actionCancelStartBeats >= 0.f, false, "Chi Animation Action Cancel 시작 Beat는 0 이상이어야 합니다.");
		}

		_settings = settings;
		return true;
	}
}
