#include "SoundWave.h"
#include "Application.h"
#include "AudioSystem.h"
#include <fmod.hpp>

namespace gm
{
	std::shared_ptr<SoundWave> SoundWave::Create(const SoundWaveDesc& desc)
	{
		FMOD::Sound* sound = nullptr;
		if (APPLICATION.GetAudioSystem().CreateSound(desc.path, &sound, desc.isLooping) == false)
			return nullptr;

		return std::shared_ptr<SoundWave>(new SoundWave(sound, desc.isLooping));
	}

	SoundWave::SoundWave(FMOD::Sound* sound, bool isLooping) : _sound(sound), _isLooping(isLooping) {}

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

}
