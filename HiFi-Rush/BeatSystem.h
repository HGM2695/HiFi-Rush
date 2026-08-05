#pragma once

#include "EngineCore.h"

namespace gm
{
	class AudioSystem;

	class BeatSystem
	{
	public:
		void	SetBPM(float bpm, float offsetSeconds = 0.f);
		void	Tick(const AudioSystem& audioSystem);

		float	GetBPM() const { return _bpm; }
		float	GetSecondsPerBeat() const { return _secondsPerBeat; }
		float	GetPlaybackTime() const { return _playbackTime; }
		float	GetCurrentBeat() const { return _currentBeat; }
		int64	GetCurrentBeatIndex() const { return _currentIntegerBeat; }
		float	GetBeatProgress() const { return _beatProgress; }
		float	GetMusicPeak() const { return _musicPeak; }
		bool	DidCrossBeatBoundary() const { return _isCrossIntegerBeat; }
		bool	HasPlaybackTime() const { return _hasPlaybackTime; }

	private:
		void	ResetPlaybackState();

	private:
		float	_bpm = 0.f;
		float	_secondsPerBeat = 0.f;
		float	_offsetSeconds = 0.f;
		float	_playbackTime = 0.f;
		float	_currentBeat = 0.f;
		int64	_currentIntegerBeat = 0;
		float	_beatProgress = 0.f;
		float	_musicPeak = 0.f;
		bool	_isCrossIntegerBeat = false;
		bool	_hasPlaybackTime = false;
	};
}
