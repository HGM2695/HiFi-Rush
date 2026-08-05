#pragma once

#include "Scene.h"

namespace gm
{
	class OutsideScene : public Scene
	{
	protected:
		virtual void OnEnter() override;
		virtual void OnExit() override;
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;

	private:
		void InitializeEnvironment();
	};
}
