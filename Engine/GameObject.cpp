#include "GameObject.h"
#include "Windows.h"

namespace gm
{
	GameObject::GameObject()
	{
	}

	GameObject::~GameObject()
	{
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

