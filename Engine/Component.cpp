#include "Component.h"

namespace gm
{
	Component::Component() = default;
	Component::~Component() = default;

	void Component::Initialize()
	{
		OnInitialize();
	}

	void Component::Update()
	{
		OnUpdate();
	}
	
	void Component::LateUpdate()
	{
		OnLateUpdate();
	}

	void Component::Render(HDC hDC)
	{
		OnRender(hDC);
	}
}


