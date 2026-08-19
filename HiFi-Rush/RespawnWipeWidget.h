#pragma once

#include "UserWidget.h"

namespace gm
{
	class Image;

	class RespawnWipeWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"RespawnWipeRoot";

		void Play();

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		void Stop();
		void ApplyAnimation(float ratio);

		Image*	_topImage = nullptr;
		Image*	_bottomImage = nullptr;
		float	_elapsedTime = 0.f;
		bool	_isPlaying = false;
	};
}
