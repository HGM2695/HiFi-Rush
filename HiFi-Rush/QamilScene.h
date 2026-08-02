#pragma once

#include "Scene.h"

namespace gm
{
	class QamilScene : public Scene
	{
	protected:
		virtual void OnEnter() override;
		virtual void OnInitialize() override;
		virtual void OnTick(float deltaTime) override;

	private:
		void InitializeEnvironment();
	};
}
