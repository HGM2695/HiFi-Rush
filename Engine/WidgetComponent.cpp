#include "WidgetComponent.h"

#include "Application.h"
#include "CameraManager.h"
#include "GameObject.h"
#include "Scene.h"
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

	void WidgetComponent::OnTick(float deltaTime)
	{
		if (_widget == nullptr)
			return;

		_widget->Tick(deltaTime);
	}

	void WidgetComponent::OnRender()
	{
		if (_widget == nullptr || _ownerTransform == nullptr)
			return;

		Scene* scene = GetOwner().GetScene();
		if (scene == nullptr || scene->GetCameraManager() == nullptr)
			return;

		const Vector2 worldPosition = _ownerTransform->GetPosition2D() + _worldOffset;
		const Vector2 screenPosition = scene->GetCameraManager()->WorldToScreen(worldPosition, APPLICATION.GetWidth(), APPLICATION.GetHeight());
		_widget->Render(screenPosition);
	}
}
