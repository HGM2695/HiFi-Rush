#include "GameObject.h"
#include "Collider2D.h"
#include "Rigidbody2D.h"
#include "Scene.h"
#include "Transform.h"

namespace gm
{
	GameObject::GameObject()
	{
		AddComponent<Transform>();
	}

	GameObject::GameObject(const Vector2& position) : GameObject()
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
			component->Render();
	}

	bool GameObject::RegisterComponent(Component* component)
	{
		if (auto transform = dynamic_cast<Transform*>(component))
		{
			GM_ASSERT_RETURN_VAL(_transform == nullptr, false, "GameObject에는 Transform이 중복으로 추가될 수 없습니다.");
			_transform = transform;
			return true;
		}

		if (auto rigidbody2D = dynamic_cast<Rigidbody2D*>(component))
		{
			GM_ASSERT_RETURN_VAL(_rigidbody2D == nullptr, false, "GameObject에는 Rigidbody2D가 중복으로 추가될 수 없습니다.");
			_rigidbody2D = rigidbody2D;
			return true;
		}

		if (auto collider2D = dynamic_cast<Collider2D*>(component))
		{
			_colliders2D.push_back(collider2D);
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

	Transform* GameObject::GetTransform()
	{
		GM_ASSERT(_transform, "GameObject는 반드시 Transform 컴포넌트를 가져야 합니다.");
		return _transform;
	}

	const Transform* GameObject::GetTransform() const
	{
		GM_ASSERT(_transform, "GameObject는 반드시 Transform 컴포넌트를 가져야 합니다.");
		return _transform;
	}
}
