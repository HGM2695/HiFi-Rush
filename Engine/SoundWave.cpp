#include "SoundWave.h"
#include "Application.h"
#include "AudioSystem.h"
#include <fmod.hpp>

namespace gm
{
	std::shared_ptr<SoundWave> SoundWave::Create(const SoundWaveDesc& desc)
	{
		FMOD::Sound* sound = nullptr;
		if (APPLICATION.GetAudioSystem().CreateSound(desc.path, &sound) == false)
			return nullptr;

		return std::shared_ptr<SoundWave>(new SoundWave(sound));
	}

	SoundWave::SoundWave(FMOD::Sound* sound) : _sound(sound) {}

	SoundWave::~SoundWave()
	{
		if (_sound)
		{
			_sound->release();
			_sound = nullptr;
		}
	}
}
