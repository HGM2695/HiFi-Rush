#include "GameObject.h"
#include "Windows.h"
#include "Transform.h"
#include "GMAssert.h"

namespace gm
{
	GameObject::GameObject()
	{
		AddComponent<Transform>();
	}

	GameObject::GameObject(const math::Vector2& position) : GameObject()
	{
		GetTransform()->SetPosition(position);
	}

	GameObject::~GameObject()
	{
	}

	void GameObject::Destroy()
	{
		if (_lifeState == GameObjectLifeState::PendingDestroy)
			return;

		_lifeState = GameObjectLifeState::PendingDestroy;
	}

	Transform* GameObject::GetTransform()
	{
		Transform* transform = GetComponent<Transform>();
		GM_ASSERT(transform, "GameObject는 반드시 Transform 컴포넌트를 가져야 합니다.");
		return transform;
	}

	const Transform* GameObject::GetTransform() const
	{
		const Transform* transform = GetComponent<Transform>();
		GM_ASSERT(transform, "GameObject는 반드시 Transform 컴포넌트를 가져야 합니다.");
		return transform;
	}

	void GameObject::Initialize()
	{
		OnInitialize();

		for (auto& component : _componentList)
			component->Initialize();
	}

	void GameObject::Update()
	{
		if (IsPendingDestroy())
			return;

		OnUpdate();

		for (auto& component : _componentList)
			component->Update();
	}

	void GameObject::LateUpdate()
	{
		if (IsPendingDestroy())
			return;

		OnLateUpdate();

		for (auto& component : _componentList)
			component->LateUpdate();
	}

	void GameObject::Render(HDC hDC)
	{
		if (IsPendingDestroy() || _isRender == false)
			return;

		OnRender(hDC);

		for (auto& component : _componentList)
			component->Render(hDC);
	}
}

