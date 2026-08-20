#pragma once

#include "UserWidget.h"

namespace gm
{
	class Image;

	class ScreenWipeWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"ScreenWipeRoot";

		void PlayOpen();
		void PlayCover();

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		enum class State
		{
			Hidden,
			Opening,
			Covering,
			Covered,
		};

		void Hide();
		void ApplyOpenRatio(float ratio);

		Image* _topImage = nullptr;
		Image* _bottomImage = nullptr;
		float _elapsedTime = 0.f;
		State _state = State::Hidden;
	};
}
