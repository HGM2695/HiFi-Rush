#pragma once

#include <memory>
#include <string>
#include "Component.h"

namespace FMOD
{
	class Channel;
}

namespace gm
{
	class SoundWave;

	class AudioComponent : public Component
	{
	public:
		explicit AudioComponent(std::wstring soundName);
		AudioComponent(const std::shared_ptr<SoundWave>& sound);
		~AudioComponent() override;

		void								SetSound(const std::wstring& soundName);
		void								SetSound(const std::shared_ptr<SoundWave>& sound) { _sound = sound; }
		const std::shared_ptr<SoundWave>&	GetSound() const { return _sound; }

		void								Play();
		void								Stop();
		void								Pause();
		void								Resume();
		void								SetPaused(bool isPaused);

		void								SetVolume(float volume);
		float								GetVolume() const { return _volume; }

		void								SetLooping(bool isLooping) { _isLooping = isLooping; }
		bool								IsLooping() const { return _isLooping; }

		void								SetAutoPlay(bool isAutoPlay) { _isAutoPlay = isAutoPlay; }
		bool								IsAutoPlay() const { return _isAutoPlay; }

		bool								IsPlaying() const;
		bool								IsPaused() const;

	protected:
		void								OnInitialize() override;
		void								OnTick(float deltaTime) override;

	private:
		std::shared_ptr<SoundWave>		_sound{};
		FMOD::Channel*					_channel = nullptr;
		float							_volume = 1.f;
		bool							_isLooping = false;
		bool							_isAutoPlay = false;
	};
}
