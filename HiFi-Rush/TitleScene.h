#pragma once

#include "../Engine/Scene.h"

namespace gm
{
	class TitleScene : public Scene
	{
	protected:
		void OnEnter() override;
		void OnUpdate() override;
		void OnRender(HDC hDC) override;
	};
}


