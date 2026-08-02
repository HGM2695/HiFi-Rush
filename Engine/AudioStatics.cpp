#include "AudioStatics.h"
#include "Application.h"
#include "Resources.h"
#include "AudioSystem.h"
#include "SoundWave.h"

namespace gm
{
	// loop 재생을 원한다면 BGM 재생이나, AudioComponent를 사용해주세요.
	void PlaySound2D(const std::wstring& soundKey, float volume)
	{
		const std::shared_ptr<SoundWave> sound = APPLICATION.GetResources().Find<SoundWave>(soundKey);
		GM_ASSERT_RETURN(sound, "키에 해당하는 사운드 웨이브 리소스가 존재하지 않습니다.");

		APPLICATION.GetAudioSystem().PlaySound2D(*sound, volume, false);
	}

	void PlayBGM(const std::wstring& soundKey, float volume)
	{
		const std::shared_ptr<SoundWave> sound = APPLICATION.GetResources().Find<SoundWave>(soundKey);
		GM_ASSERT_RETURN(sound, "키에 해당하는 사운드 웨이브 리소스가 존재하지 않습니다.");

		APPLICATION.GetAudioSystem().PlayBGM(*sound, volume, false);
	}

	void StopBGM()
	{
		APPLICATION.GetAudioSystem().StopBGM();
	}
}
