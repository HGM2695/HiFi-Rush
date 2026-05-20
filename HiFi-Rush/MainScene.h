#pragma once

#include "Scene.h"

namespace gm
{
	class GameObject;

	class MainScene : public Scene
	{
	protected:
		virtual void	OnEnter() override;
		virtual void	OnInitialize() override;

	private:
		void			InitializePlayer();
		void			InitializeSubObject();
		void			InitializeCamera(GameObject* player);
	};
}


