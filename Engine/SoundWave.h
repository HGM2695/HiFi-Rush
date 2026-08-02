#pragma once

#include "Resource.h"

namespace FMOD
{
	class Sound;
}

namespace gm
{
	struct SoundWaveDesc
	{
		std::wstring path;
	};

	class SoundWave : public Resource
	{
	friend class AudioSystem;

	public:
		static std::shared_ptr<SoundWave> Create(const SoundWaveDesc& desc);
		~SoundWave() override;

		static constexpr ResourceType Type = ResourceType::Audio;
		ResourceType	GetType() const override { return Type; }

	private:
		explicit SoundWave(FMOD::Sound* sound);
		// FMOD 의존성이 클라이언트 코드로 새지 않도록 내부 핸들은 오디오 시스템에만 접근 허용
		FMOD::Sound*	GetSound() const { return _sound; }

	private:
		FMOD::Sound*	_sound = nullptr;
	};
}
