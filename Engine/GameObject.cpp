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

	GameObject::~GameObject()
	{
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
		OnUpdate();

		for (auto& component : _componentList)
			component->Update();
	}

	void GameObject::LateUpdate()
	{
		OnLateUpdate();

		for (auto& component : _componentList)
			component->LateUpdate();
	}

	void GameObject::Render(HDC hDC)
	{
		OnRender(hDC);

		for (auto& component : _componentList)
			component->Render(hDC);
	}
}

