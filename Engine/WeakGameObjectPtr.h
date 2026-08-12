#pragma once

#include "GameObjectHandle.h"
#include <cstddef>

namespace gm
{
	class GameObject;
	class Scene;
	struct WeakGameObjectPtrHasher;

	class WeakGameObjectPtr
	{
	public:
		WeakGameObjectPtr() = default;

		GameObject*			Get() const;
		GameObject*			GetUnsafe() const { return _gameObject; }
		bool				IsValid() const;
		void				Reset();

		GameObject* operator->() const;
		explicit operator bool() const { return IsValid(); }

		bool operator==(const WeakGameObjectPtr& rhs) const
		{
			return _scene == rhs._scene && _handle == rhs._handle;
		}

		bool operator!=(const WeakGameObjectPtr& rhs) const
		{
			return !(*this == rhs);
		}

	private:
		friend class GameObject;
		friend struct WeakGameObjectPtrHasher;

		WeakGameObjectPtr(Scene* scene, GameObject* gameObject, GameObjectHandle handle);

		Scene*				_scene = nullptr;
		GameObject*			_gameObject = nullptr;
		GameObjectHandle	_handle{};
	};

	struct WeakGameObjectPtrHasher
	{
		size_t operator()(const WeakGameObjectPtr& value) const;
	};
}
