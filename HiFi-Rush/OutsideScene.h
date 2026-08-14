#pragma once

#include "GameplayScene.h"

namespace gm
{
	class OutsideScene : public GameplayScene
	{
	protected:
		virtual void OnEnter() override;
		virtual void OnExit() override;
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;
	};
}
