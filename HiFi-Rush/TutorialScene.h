#pragma once

#include "GameplayScene.h"

namespace gm
{
	class TutorialScene : public GameplayScene
	{
	protected:
		virtual void	OnEnter() override;
		virtual void	OnExit() override;
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

	private:
		void			InitializeSubObject();
		void			InitializeStaticMeshTest();
		void			InitializeSkeletalMeshTest();
	};
}


