#include "BeatSystem.h"
#include "AudioSystem.h"
#include <cmath>

namespace gm
{
	void BeatSystem::SetBPM(float bpm, float offsetSeconds)
	{
		GM_ASSERT_RETURN(bpm > 0.f, "BPM은 0보다 커야 합니다.");

		_bpm = bpm;
		_secondsPerBeat = 60.f / bpm;
		_offsetSeconds = offsetSeconds;
		ResetPlaybackState();
	}

	void BeatSystem::Tick(const AudioSystem& audioSystem)
	{
		_isCrossIntegerBeat = false;

		float playbackTime = 0.f;
		if (_secondsPerBeat <= 0.f || audioSystem.GetBGMPlayTime(playbackTime) == false)
		{
			ResetPlaybackState();
			return;
		}

		const int64 previousBeatIndex = _currentIntegerBeat;

		_playbackTime = playbackTime;
		_currentBeat = (_playbackTime + _offsetSeconds) / _secondsPerBeat;
		const float currentBeatFloor = std::floor(_currentBeat);
		_currentIntegerBeat = static_cast<int64>(currentBeatFloor);
		_beatProgress = _currentBeat - currentBeatFloor;
		_isCrossIntegerBeat = _hasPlaybackTime && _currentIntegerBeat != previousBeatIndex;

		if (_hasPlaybackTime == false || _currentIntegerBeat != previousBeatIndex)
		{
			float musicPeak = 0.f;
			if (audioSystem.GetBGMPeak(musicPeak))
				_musicPeak = musicPeak;
		}

		_hasPlaybackTime = true;
	}

	void BeatSystem::ResetPlaybackState()
	{
		_playbackTime = 0.f;
		_currentBeat = 0.f;
		_currentIntegerBeat = 0;
		_beatProgress = 0.f;
		_musicPeak = 0.f;
		_isCrossIntegerBeat = false;
		_hasPlaybackTime = false;
	}
}
