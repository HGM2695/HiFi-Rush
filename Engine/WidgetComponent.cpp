#include "WidgetComponent.h"

#include "Application.h"
#include "CameraManager.h"
#include "GameObject.h"
#include "Scene.h"
#include "TransformComponent.h"
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
		_ownerTransform = GetOwner().GetComponent<TransformComponent>();

		_widget->Initialize();
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
		if (scene == nullptr)
			return;

		CameraManager* cameraManager = scene->GetCameraManager();
		if (cameraManager == nullptr)
			return;

		const Vector3 worldPosition = _ownerTransform->GetPosition();
		const Vector2 screenPosition = cameraManager->WorldToScreen(worldPosition, APPLICATION.GetWidth(), APPLICATION.GetHeight());

		const Widget* rootWidget = _widget->GetRootWidget();
		_widget->Render(WidgetGeometry{ screenPosition, rootWidget ? rootWidget->GetSize() : Vector2{} });
	}
}
