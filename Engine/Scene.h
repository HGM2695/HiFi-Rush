#pragma once

#include <vector>
#include <memory>
#include "Entity.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class GameObject;

	class Scene : public Entity
	{
	public:
		Scene();
		virtual ~Scene();

		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC);

		void			AddGameObject(std::unique_ptr<GameObject> gameObject);

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUpdate() {}
		virtual void	OnLateUpdate() {}
		virtual void	OnRender(HDC hDC) {}
		
	private:
		std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	};
}


