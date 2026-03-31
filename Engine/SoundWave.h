#pragma once

#include "Resource.h"

namespace FMOD
{
	class Sound;
}

namespace gm
{
	class SoundWave : public Resource
	{
	friend class AudioSystem;

	public:
		static constexpr ResourceType Type = ResourceType::Audio;

		SoundWave() = default;
		~SoundWave() override;

		ResourceType	GetType() const override { return Type; }

		void			SetLooping(bool isLooping);
		bool			IsLooping() const { return _isLooping; }

	protected:
		bool			LoadInternal(const std::wstring& path) override;

	private:
		// FMOD 의존성이 클라이언트 코드로 새지 않도록 내부 핸들은 오디오 시스템에만 접근 허용
		FMOD::Sound*	GetSound() const { return _sound; }

	private:
		FMOD::Sound*	_sound = nullptr;
		bool			_isLooping = false;
	};
}
