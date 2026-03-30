#include "AudioComponent.h"
#include "Application.h"
#include "AudioSystem.h"
#include "Resources.h"
#include "GMAssert.h"
#include "SoundWave.h"
#include <algorithm>
#include <fmod.hpp>

namespace gm
{
	AudioComponent::AudioComponent(std::wstring soundName)
	{
		SetSound(soundName);
	}

	AudioComponent::AudioComponent(const std::shared_ptr<SoundWave>& sound)
	{
		SetSound(sound);
	}

	AudioComponent::~AudioComponent()
	{
		Stop();
	}

	void AudioComponent::SetSound(const std::wstring& soundName)
	{
		auto sound = APPLICATION.GetResources().Find<SoundWave>(soundName);
		GM_ASSERT_RETURN(sound, "%ls 를 Key로 하는 SoundWave를 찾을 수 없습니다.", soundName);

		_sound = sound;
	}

	void AudioComponent::OnInitialize()
	{
		if (_isAutoPlay)
			Play();
	}

	void AudioComponent::OnUpdate()
	{
		if (_channel == nullptr)
			return;

		bool isPlaying = false;
		const FMOD_RESULT result = _channel->isPlaying(&isPlaying);
		if (result != FMOD_OK || isPlaying == false)
		{
			_channel = nullptr;
			return;
		}

		_channel->setVolume(_volume);
	}

	void AudioComponent::Play()
	{
		GM_ASSERT_RETURN(_sound, "AudioComponent는 재생할 SoundWave가 필요합니다.");

		Stop();
		_channel = APPLICATION.GetAudioSystem().PlaySound2D(*_sound, _volume, _isLooping);
	}

	void AudioComponent::Stop()
	{
		if (_channel == nullptr)
			return;

		APPLICATION.GetAudioSystem().StopChannel(_channel);
		_channel = nullptr;
	}

	void AudioComponent::Pause()
	{
		SetPaused(true);
	}

	void AudioComponent::Resume()
	{
		SetPaused(false);
	}

	void AudioComponent::SetPaused(bool isPaused)
	{
		if (_channel == nullptr)
			return;

		_channel->setPaused(isPaused);
	}

	void AudioComponent::SetVolume(float volume)
	{
		_volume = std::clamp(volume, 0.f, 1.f);

		if (_channel)
			_channel->setVolume(_volume);
	}

	bool AudioComponent::IsPlaying() const
	{
		if (_channel == nullptr)
			return false;

		bool isPlaying = false;
		return _channel->isPlaying(&isPlaying) == FMOD_OK && isPlaying;
	}

	bool AudioComponent::IsPaused() const
	{
		if (_channel == nullptr)
			return false;

		bool isPaused = false;
		return _channel->getPaused(&isPaused) == FMOD_OK && isPaused;
	}
}
