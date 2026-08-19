#include "HiFiRushAudio.h"
#include "Application.h"
#include "AudioSystem.h"
#include "BeatSystem.h"
#include "HiFiRushStatics.h"
#include "Resources.h"
#include "SoundWave.h"

namespace gm
{
	namespace
	{
		bool PlayBGM(const RhythmBGMDesc& desc)
		{
			const std::shared_ptr<SoundWave> sound = APPLICATION.GetResources().Find<SoundWave>(desc.commonResourceKey);
			GM_ASSERT_RETURN_VAL(sound, false, "BGM SoundWave 리소스가 존재하지 않습니다. key=%ls", desc.commonResourceKey);

			FMOD::Channel* channel = APPLICATION.GetAudioSystem().PlayBGM(*sound, desc.volume);
			GM_ASSERT_RETURN_VAL(channel, false, "BGM 재생에 실패했습니다. key=%ls", desc.commonResourceKey);
			return true;
		}

		void StartRhythmBGM(const RhythmBGMDesc& desc, float songOffsetSeconds, float timelineOffsetSeconds)
		{
			if (PlayBGM(desc) == false)
				return;

			HiFiRushStatics::GetBeatSystem().SetBPM(desc.bpm, timelineOffsetSeconds + songOffsetSeconds);
		}
	}

	void PlayRhythmBGM(const RhythmBGMDesc& desc)
	{
		StartRhythmBGM(desc, desc.songOffsetSeconds, 0.f);
	}

	void TransitionRhythmBGM(const RhythmBGMDesc& desc)
	{
		GM_ASSERT_RETURN(desc.bpm > 0.f, "BGM BPM은 0보다 커야 합니다. key=%ls", desc.commonResourceKey);
		BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		const float currentBeat = beatSystem.GetCurrentBeat();
		const float continuedBeatOffset = currentBeat * (60.f / desc.bpm);
		StartRhythmBGM(desc, desc.songOffsetSeconds, continuedBeatOffset);
	}
}
