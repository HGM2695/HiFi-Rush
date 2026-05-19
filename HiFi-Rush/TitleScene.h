#pragma once

#include "Scene.h"

namespace gm
{
	class TitleScene : public Scene
	{
	protected:
		void OnEnter() override;
		void OnTick(float deltaTime) override;
		void OnRender() override;
	};
}


