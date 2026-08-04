#pragma once

#include <string>
#include <vector>

namespace FMOD
{
	class System;
	class Channel;
	class Sound;
}

namespace gm
{
	class SoundWave;

	class AudioSystem
	{
	public:
		AudioSystem() = default;
		~AudioSystem();

		bool			Initialize();
		void			Tick();
		void			ShutDown();

		bool			IsInitialized() const { return _system != nullptr; }

		bool			CreateSound(const std::wstring& path, _Out_ FMOD::Sound** outSound);
		FMOD::Channel*	PlaySound2D(const SoundWave& sound, float volume = 1.f, bool isLooping = false, bool startPaused = false);
		FMOD::Channel*	PlayBGM(const SoundWave& sound, float volume = 1.f, bool startPaused = false);
		bool			GetBGMPlayTime(_Out_ float& outPlaybackTimeSeconds) const;
		void			StopChannel(FMOD::Channel* channel);
		void			StopBGM();
		void			StopAllSounds(bool includeBGM);

	private:
		FMOD::Channel*	PlaySound2DInternal(const SoundWave& sound, float volume = 1.f, bool isLooping = false, bool startPaused = false);
		void			RemoveStoppedChannels();

	private:
		FMOD::System*				_system = nullptr;
		FMOD::Channel*				_bgmChannel = nullptr;
		std::vector<FMOD::Channel*> _activeChannels;
	};
}
