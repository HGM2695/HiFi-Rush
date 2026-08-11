#include "GameObject.h"
#include "Collider2DComponent.h"
#include "Collider3DComponent.h"
#include "Rigidbody2DComponent.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "TransformComponent.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	GameObject::GameObject()
	{
		AddComponent<TransformComponent>();
	}

	GameObject::GameObject(const Vector3& position) : GameObject()
	{
		GetTransform()->SetPosition(position);
	}

	GameObject::~GameObject() {}

	void GameObject::Initialize()
	{
		if (_isInitialized)
			return;

		OnInitialize();

		for (auto& component : _componentList)
			component->Initialize();

		_isInitialized = true;
	}

	void GameObject::Tick(float deltaTime)
	{
		if (_isInitialized == false || IsPendingDestroy())
			return;

		OnTick(deltaTime);
	}

	void GameObject::Render()
	{
		if (_isInitialized == false || IsPendingDestroy() || _isRender == false)
			return;

		OnRender();

		for (auto& component : _componentList)
		{
			if (component->IsEnabled() == false)
				continue;

			component->Render();
		}
	}

	bool GameObject::RegisterComponent(Component* component)
	{
		if (auto transform = dynamic_cast<TransformComponent*>(component))
		{
			GM_ASSERT_RETURN_VAL(_transform == nullptr, false, "GameObject에는 Transform이 중복으로 추가될 수 없습니다.");
			_transform = transform;
			return true;
		}

		if (auto rigidbody2D = dynamic_cast<Rigidbody2DComponent*>(component))
		{
			GM_ASSERT_RETURN_VAL(_rigidbody2D == nullptr, false, "GameObject에는 Rigidbody2D가 중복으로 추가될 수 없습니다.");
			_rigidbody2D = rigidbody2D;
			return true;
		}

		if (auto rigidbody3D = dynamic_cast<Rigidbody3DComponent*>(component))
		{
			GM_ASSERT_RETURN_VAL(_rigidbody3D == nullptr, false, "GameObject에는 Rigidbody3D가 중복으로 추가될 수 없습니다.");
			_rigidbody3D = rigidbody3D;
			return true;
		}

		if (auto collider2D = dynamic_cast<Collider2DComponent*>(component))
		{
			_colliders2D.push_back(collider2D);
			return true;
		}

		if (auto collider3D = dynamic_cast<Collider3DComponent*>(component))
		{
			_colliders3D.push_back(collider3D);
			return true;
		}

		return true;
	}

	void GameObject::NotifyComponentAdded(Component& component)
	{
		if (_scene)
			_scene->NotifyComponentAdded(component);
	}

	void GameObject::Destroy()
	{
		if (_lifeState == GameObjectLifeState::PendingDestroy)
			return;

		_lifeState = GameObjectLifeState::PendingDestroy;
	}

	WeakGameObjectPtr GameObject::GetWeakPtr() const
	{
		return WeakGameObjectPtr(_scene, const_cast<GameObject*>(this), _handle);
	}

	TransformComponent* GameObject::GetTransform()
	{
		GM_ASSERT(_transform, "GameObject는 반드시 TransformComponent 컴포넌트를 가져야 합니다.");
		return _transform;
	}

	const TransformComponent* GameObject::GetTransform() const
	{
		GM_ASSERT(_transform, "GameObject는 반드시 TransformComponent 컴포넌트를 가져야 합니다.");
		return _transform;
	}
}
