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

		for (auto& component : _ComponentList)
			component->Initialize();
	}

	void GameObject::Update()
	{
		OnUpdate();

		for (auto& component : _ComponentList)
			component->Update();
	}

	void GameObject::LateUpdate()
	{
		OnLateUpdate();

		for (auto& component : _ComponentList)
			component->LateUpdate();
	}

	void GameObject::Render(HDC hDC)
	{
		OnRender(hDC);

		for (auto& component : _ComponentList)
			component->Render(hDC);
	}
}

