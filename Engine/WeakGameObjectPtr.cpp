#include "WeakGameObjectPtr.h"
#include "Scene.h"

namespace gm
{
	WeakGameObjectPtr::WeakGameObjectPtr(Scene* scene, GameObject* gameObject, GameObjectHandle handle)
		: _scene(scene), _gameObject(gameObject), _handle(handle) { }

	GameObject* WeakGameObjectPtr::Get() const
	{
		if (IsValid() == false)
			return nullptr;

		return _gameObject;
	}

	bool WeakGameObjectPtr::IsValid() const
	{
		return _scene != nullptr && _scene->FindGameObject(_handle) == _gameObject;
	}

	void WeakGameObjectPtr::Reset()
	{
		_scene = nullptr;
		_gameObject = nullptr;
		_handle = {};
	}

	GameObject* WeakGameObjectPtr::operator->() const
	{
		GM_ASSERT_RETURN_VAL(IsValid(), nullptr, "유효하지 않은 WeakGameObjectPtr입니다.");
		return GetUnsafe();
	}
}
