#include "SoundWave.h"
#include "Application.h"
#include "GMAssert.h"
#include "AudioSystem.h"
#include <fmod.hpp>

namespace gm
{
	SoundWave::~SoundWave()
	{
		if (_sound)
		{
			_sound->release();
			_sound = nullptr;
		}
	}

	void SoundWave::SetLooping(bool isLooping)
	{
		_isLooping = isLooping;

		if (_sound)
		{
			const FMOD_MODE mode = _isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
			_sound->setMode(mode);
		}
	}

	bool SoundWave::Load(const std::wstring& path)
	{
		FMOD::Sound* sound = nullptr;
		if (APPLICATION.GetAudioSystem().CreateSound(path, &sound, _isLooping) == false)
			return false;

		_sound = sound;
		return true;
	}
}
