#pragma once

#include "Scene.h"

namespace gm
{
	class CommonLoadingScene : public Scene
	{
		
	protected:
		void OnEnter() override;
		void OnUpdate() override;
		void OnRender(HDC hDC) override;

	private:
		std::wstring _pendingSceneName{};

	};

}