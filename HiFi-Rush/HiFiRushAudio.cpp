#include "HiFiRushAudio.h"
#include "Application.h"
#include "AudioSystem.h"
#include "BeatSystem.h"
#include "HiFiRushStatics.h"
#include "Resources.h"
#include "SoundWave.h"

namespace gm
{
	void PlayRhythmBGM(const RhythmBGMDesc& desc)
	{
		const std::shared_ptr<SoundWave> sound = APPLICATION.GetResources().Find<SoundWave>(desc.resourceKey);
		GM_ASSERT_RETURN(sound, "BGM SoundWave 리소스가 존재하지 않습니다. key=%ls", desc.resourceKey);

		FMOD::Channel* channel = APPLICATION.GetAudioSystem().PlayBGM(*sound, desc.volume);
		GM_ASSERT_RETURN(channel, "BGM 재생에 실패했습니다. key=%ls", desc.resourceKey);

		HiFiRushStatics::GetBeatSystem().SetBPM(desc.bpm, desc.playbackOffsetSeconds);
	}
}
