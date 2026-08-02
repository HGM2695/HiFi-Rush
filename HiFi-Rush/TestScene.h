#pragma once

#include "Scene.h"

namespace gm
{
	class CameraComponent;
	class GameObject;

	class TestScene : public Scene
	{
	protected:
		virtual void	OnEnter() override;
		virtual void	OnExit() override;
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

	private:
		void			InitializePlayer();
		void			InitializeSubObject();
		void			InitializeStaticMeshTest();
		void			InitializeSkeletalMeshTest();
		CameraComponent*	InitializeCamera(GameObject* player);

#if GM_ENABLE_DEBUG_TOOLS
		void			TickCameraProjectionDebug(float deltaTime);
#endif
	};
}
