#pragma once

#include "UserWidget.h"
#include <vector>

namespace gm
{
	class Image;
	class Texture;

	class TitleWidget : public UserWidget
	{
	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		Image* _background = nullptr;
		std::vector<std::shared_ptr<Texture>> _frames{};
		float _elapsedTime = 0.f;
		uint32 _currentFrameIndex = 0;
	};
}
