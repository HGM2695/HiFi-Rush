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
		
	private:
		std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	};
}


