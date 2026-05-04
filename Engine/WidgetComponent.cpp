#include "WidgetComponent.h"

#include "Camera.h"
#include "GameObject.h"
#include "Transform.h"
#include "UserWidget.h"

namespace gm
{
	void WidgetComponent::SetWidgetVisible(bool isVisible)
	{
		if (_widget == nullptr)
			return;

		_widget->SetVisible(isVisible);
	}

	bool WidgetComponent::IsWidgetVisible() const
	{
		if (_widget == nullptr)
			return false;

		return _widget->IsVisible();
	}

	void WidgetComponent::ToggleWidgetVisibility()
	{
		if (_widget == nullptr)
			return;

		_widget->ToggleVisibility();
	}

	void WidgetComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<Transform>();
	}

	void WidgetComponent::OnUpdate()
	{
		if (_widget == nullptr)
			return;

		_widget->Update();
	}

	void WidgetComponent::OnRender(HDC hDC)
	{
		if (_widget == nullptr || _ownerTransform == nullptr)
			return;

		const Vector2 worldPosition = _ownerTransform->GetPosition() + _worldOffset;
		const Vector2 screenPosition = Camera::MainWorldToScreen(worldPosition);
		_widget->Render(hDC, screenPosition);
	}
}
