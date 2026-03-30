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
	public:
		static constexpr ResourceType Type = ResourceType::Audio;

		SoundWave() = default;
		~SoundWave() override;

		ResourceType	GetType() const override { return Type; }

		void			SetLooping(bool isLooping);
		bool			IsLooping() const { return _isLooping; }

		FMOD::Sound*	GetSound() const { return _sound; }

	protected:
		bool			LoadInternal(const std::wstring& path) override;

	private:
		FMOD::Sound*	_sound = nullptr;
		bool			_isLooping = false;
	};
}
