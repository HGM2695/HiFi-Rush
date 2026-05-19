#pragma once

#include "GameObjectHandle.h"

namespace gm
{
	class GameObject;
	class Scene;

	class WeakGameObjectPtr
	{
	public:
		WeakGameObjectPtr() = default;
		WeakGameObjectPtr(Scene* scene, GameObject* gameObject, GameObjectHandle handle);

		GameObject*			Get() const;
		GameObject*			GetUnsafe() const { return _gameObject; }
		bool				IsValid() const;
		void				Reset();
		GameObjectHandle	GetHandle() const { return _handle; }

		GameObject* operator->() const;
		explicit operator bool() const { return IsValid(); }

	private:
		Scene*				_scene = nullptr;
		GameObject*			_gameObject = nullptr;
		GameObjectHandle	_handle{};
	};
}
