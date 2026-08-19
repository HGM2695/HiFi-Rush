#include "AudioSystem.h"
#include "GMAssert.h"
#include "GMLog.h"
#include "SoundWave.h"
#include "StringUtil.h"
#include <algorithm>
#include <fmod.hpp>
#include <fmod_errors.h>

namespace
{
	constexpr unsigned int DSPBufferLength = 512;
	constexpr int DSPBufferCount = 4;

	void LogFMODError(const char* context, FMOD_RESULT result)
	{
		GM_LOG_ERROR("%s failed: %s", context, FMOD_ErrorString(result));
	}
}

namespace gm
{
	AudioSystem::~AudioSystem()
	{
		ShutDown();
	}

	bool AudioSystem::Initialize()
	{
		if (_system)
			return true;

		FMOD_RESULT result = FMOD::System_Create(&_system);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::System_Create", result);
			_system = nullptr;
			return false;
		}

		result = _system->setDSPBufferSize(DSPBufferLength, DSPBufferCount);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::System::setDSPBufferSize", result);
			_system->release();
			_system = nullptr;
			return false;
		}

		result = _system->init(512, FMOD_INIT_NORMAL, nullptr);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::System::init", result);
			_system->release();
			_system = nullptr;
			return false;
		}

		return true;
	}

	void AudioSystem::Tick()
	{
		if (_system == nullptr)
			return;

		RemoveStoppedChannels();

		const FMOD_RESULT result = _system->update();
		if (result != FMOD_OK)
			LogFMODError("FMOD::System::update", result);
	}

	void AudioSystem::ShutDown()
	{
		if (_system == nullptr)
			return;

		StopAllSounds(true);
		_bgmChannel = nullptr;

		_system->close();
		_system->release();
		_system = nullptr;
	}

	bool AudioSystem::CreateSound(const std::wstring& path, _Out_ FMOD::Sound** outSound)
	{
		GM_ASSERT_RETURN_VAL(outSound, false, "outSound는 nullptr일 수 없습니다.");
		*outSound = nullptr;

		if (_system == nullptr)
		{
			GM_LOG_ERROR("AudioSystem is not initialized.");
			return false;
		}

		const std::string utf8Path = gm::WideToUtf8(path);
		GM_ASSERT_RETURN_VAL(utf8Path.empty() == false, false, "오디오 경로 UTF-8 변환에 실패했습니다.");

		const FMOD_RESULT result = _system->createSound(utf8Path.c_str(), FMOD_DEFAULT, nullptr, outSound);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::System::createSound", result);
			return false;
		}

		return true;
	}

	FMOD::Channel* AudioSystem::PlaySound2D(const SoundWave& sound, float volume, bool isLooping, bool startPaused)
	{
		FMOD::Channel* channel = PlaySound2DInternal(sound, volume, isLooping, startPaused);

		if (channel)
			_activeChannels.push_back(channel);

		return channel;
	}

	FMOD::Channel* AudioSystem::PlayBGM(const SoundWave& sound, float volume, bool startPaused)
	{
		StopBGM();

		_bgmChannel = PlaySound2DInternal(sound, volume, true, startPaused);
		if (_bgmChannel)
			EnableBGMMetering();

		return _bgmChannel;
	}

	bool AudioSystem::GetBGMPlayTime(_Out_ float& outPlaybackTimeSeconds) const
	{
		outPlaybackTimeSeconds = 0.f;

		if (_bgmChannel == nullptr)
			return false;

		unsigned int playbackTimeMilliseconds = 0;
		const FMOD_RESULT result = _bgmChannel->getPosition(&playbackTimeMilliseconds, FMOD_TIMEUNIT_MS);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::Channel::getPosition", result);
			return false;
		}

		outPlaybackTimeSeconds = static_cast<float>(playbackTimeMilliseconds) / 1000.f;
		return true;
	}

	bool AudioSystem::GetBGMPeak(_Out_ float& outPeak) const
	{
		outPeak = 0.f;

		if (_bgmChannel == nullptr)
			return false;

		FMOD::DSP* meterDSP = nullptr;
		FMOD_RESULT result = _bgmChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &meterDSP);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::Channel::getDSP", result);
			return false;
		}

		FMOD_DSP_METERING_INFO outputInfo{};
		result = meterDSP->getMeteringInfo(nullptr, &outputInfo);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::DSP::getMeteringInfo", result);
			return false;
		}

		if (outputInfo.numchannels <= 0)
			return false;

		for (int channelIndex = 0; channelIndex < outputInfo.numchannels; ++channelIndex)
			outPeak = std::max(outPeak, outputInfo.peaklevel[channelIndex]);

		return true;
	}

	void AudioSystem::SetSpectrumAnalysisEnabled(FMOD::Channel* channel, bool isEnabled)
	{
		if (channel == nullptr)
			return;

		const auto iter = _spectrumAnalyzers.find(channel);
		if (isEnabled)
		{
			if (iter != _spectrumAnalyzers.end())
				return;

			FMOD::DSP* analyzer = nullptr;
			FMOD_RESULT result = _system->createDSPByType(FMOD_DSP_TYPE_FFT, &analyzer);
			if (result != FMOD_OK || analyzer == nullptr)
			{
				LogFMODError("FMOD::System::createDSPByType", result);
				return;
			}

			result = analyzer->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 512);
			if (result == FMOD_OK)
				result = channel->addDSP(0, analyzer);
			if (result != FMOD_OK)
			{
				LogFMODError("FMOD::Channel::addDSP", result);
				analyzer->release();
				return;
			}

			_spectrumAnalyzers.insert({ channel, analyzer });
			return;
		}

		if (iter == _spectrumAnalyzers.end())
			return;

		channel->removeDSP(iter->second);
		iter->second->release();
		_spectrumAnalyzers.erase(iter);
	}

	float AudioSystem::GetSpectrumAmplitude(FMOD::Channel* channel) const
	{
		const auto iter = _spectrumAnalyzers.find(channel);
		if (iter == _spectrumAnalyzers.end())
			return 0.f;

		FMOD_DSP_PARAMETER_FFT* fftData = nullptr;
		if (iter->second->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, reinterpret_cast<void**>(&fftData), nullptr, nullptr, 0) != FMOD_OK || fftData == nullptr)
			return 0.f;

		float amplitude = 0.f;
		for (int channelIndex = 0; channelIndex < fftData->numchannels; ++channelIndex)
		{
			for (int bandIndex = 0; bandIndex < fftData->length; ++bandIndex)
				amplitude += fftData->spectrum[channelIndex][bandIndex];
		}

		return amplitude;
	}

	void AudioSystem::StopChannel(FMOD::Channel* channel)
	{
		if (channel == nullptr)
			return;

		SetSpectrumAnalysisEnabled(channel, false);
		channel->stop();

		if (_bgmChannel == channel)
			_bgmChannel = nullptr;

		_activeChannels.erase(
			std::remove(_activeChannels.begin(), _activeChannels.end(), channel),
			_activeChannels.end());
	}

	void AudioSystem::StopBGM()
	{
		if (_bgmChannel == nullptr)
			return;

		_bgmChannel->stop();
		_bgmChannel = nullptr;
	}

	void AudioSystem::StopAllSounds(bool includeBGM)
	{
		for (FMOD::Channel* channel : _activeChannels)
		{
			if (channel == nullptr)
				continue;

			SetSpectrumAnalysisEnabled(channel, false);
			channel->stop();
		}

		_activeChannels.clear();

		if (includeBGM)
			StopBGM();
	}

	FMOD::Channel* AudioSystem::PlaySound2DInternal(const SoundWave& sound, float volume, bool isLooping, bool startPaused)
	{
		if (_system == nullptr)
		{
			GM_LOG_ERROR("AudioSystem is not initialized.");
			return nullptr;
		}

		FMOD::Sound* soundHandle = sound.GetSound();
		if (soundHandle == nullptr)
		{
			GM_LOG_ERROR("SoundWave is not loaded.");
			return nullptr;
		}

		FMOD::Channel* channel = nullptr;
		const FMOD_RESULT playResult = _system->playSound(soundHandle, nullptr, startPaused, &channel);
		if (playResult != FMOD_OK)
		{
			LogFMODError("FMOD::System::playSound", playResult);
			return nullptr;
		}

		const FMOD_MODE mode = isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		channel->setMode(mode);
		channel->setVolume(volume);

		if (startPaused == false)
			channel->setPaused(false);

		return channel;
	}

	bool AudioSystem::EnableBGMMetering()
	{
		if (_bgmChannel == nullptr)
			return false;

		FMOD::DSP* meterDSP = nullptr;
		FMOD_RESULT result = _bgmChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &meterDSP);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::Channel::getDSP", result);
			return false;
		}

		result = meterDSP->setMeteringEnabled(false, true);
		if (result != FMOD_OK)
		{
			LogFMODError("FMOD::DSP::setMeteringEnabled", result);
			return false;
		}

		return true;
	}

	void AudioSystem::RemoveStoppedChannels()
	{
		if (_bgmChannel != nullptr)
		{
			bool isPlaying = false;
			const FMOD_RESULT result = _bgmChannel->isPlaying(&isPlaying);
			if (result != FMOD_OK || isPlaying == false)
				_bgmChannel = nullptr;
		}

		_activeChannels.erase(
			std::remove_if(_activeChannels.begin(), _activeChannels.end(),
				[this](FMOD::Channel* channel)
				{
					if (channel == nullptr)
						return true;

					bool isPlaying = false;
					const FMOD_RESULT playResult = channel->isPlaying(&isPlaying);
					const bool hasStopped = playResult != FMOD_OK || isPlaying == false;
					if (hasStopped)
						SetSpectrumAnalysisEnabled(channel, false);

					return hasStopped;
				}),
			_activeChannels.end());
	}
}
