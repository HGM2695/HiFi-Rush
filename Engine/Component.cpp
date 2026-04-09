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

	GameObject& Component::GetOwner()
	{
		GM_ASSERT_TERMINATE(_owner, "컴포넌트는 항상 소유자가 있어야 합니다.");
		return *_owner;
	}

	const GameObject& Component::GetOwner() const
	{
		GM_ASSERT_TERMINATE(_owner, "컴포넌트는 항상 소유자가 있어야 합니다.");
		return *_owner;
	}
}


