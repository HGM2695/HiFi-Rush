#pragma once

#include "Scene.h"

namespace gm
{
	class CommonLoadingScene : public Scene
	{
		
	protected:
		void OnEnter() override;
		void OnTick(float deltaTime) override;
		void OnRender() override;

	private:
		std::wstring _pendingSceneName{};
	};
}
