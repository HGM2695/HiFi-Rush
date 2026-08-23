#include "EnvironmentModelDebugLabelComponent.h"

#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "CameraManager.h"
#include "GameObject.h"
#include "ITextRenderer.h"
#include "Scene.h"
#include "SkeletalMeshComponent.h"
#include "StaticMeshComponent.h"
#include "TextTypes.h"
#include "TransformComponent.h"

namespace gm
{
	EnvironmentModelDebugLabelComponent::EnvironmentModelDebugLabelComponent(uint32 modelIndex)
		: _modelIndex(modelIndex), _label(L"Model " + std::to_wstring(modelIndex))
	{}

	void EnvironmentModelDebugLabelComponent::ToggleLabels()
	{
		_areLabelsVisible = !_areLabelsVisible;
	}

	void EnvironmentModelDebugLabelComponent::OnInitialize()
	{
		GameObject& owner = GetOwner();
		_transform = owner.GetTransform();
		_staticMesh = owner.GetComponent<StaticMeshComponent>();
		_skeletalMesh = owner.GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN(_transform && (_staticMesh || _skeletalMesh), "Environment Model Debug Label에 Mesh Component가 없습니다.");
	}

	void EnvironmentModelDebugLabelComponent::OnRender()
	{
		if (_areLabelsVisible == false || _transform == nullptr)
			return;

		Scene* scene = GetOwner().GetScene();
		CameraManager* cameraManager = scene ? scene->GetCameraManager() : nullptr;
		if (cameraManager == nullptr || cameraManager->GetActiveCamera() == nullptr)
			return;

		const BoundingVolume worldBounds = GetWorldBounds();
		const Vector3 labelPosition = worldBounds.isValid ? Vector3{ worldBounds.box.Center.x, worldBounds.box.Center.y, worldBounds.box.Center.z } : _transform->GetPosition();
		const CameraViewInfo viewInfo = cameraManager->GetViewInfo();
		Vector4 clipPosition = Vector4::Transform(Vector4{ labelPosition.x, labelPosition.y, labelPosition.z, 1.f }, viewInfo.view * viewInfo.projection);
		if (clipPosition.w <= 0.f)
			return;

		const float ndcX = clipPosition.x / clipPosition.w;
		const float ndcY = clipPosition.y / clipPosition.w;
		const float ndcZ = clipPosition.z / clipPosition.w;
		if (ndcX < -1.f || ndcX > 1.f || ndcY < -1.f || ndcY > 1.f || ndcZ < 0.f || ndcZ > 1.f)
			return;

		const uint32 screenWidth = APPLICATION.GetWidth();
		const uint32 screenHeight = APPLICATION.GetHeight();
		const Vector2 screenPosition{ (ndcX + 1.f) * 0.5f * static_cast<float>(screenWidth), (1.f - ndcY) * 0.5f * static_cast<float>(screenHeight) };

		ITextRenderer& textRenderer = APPLICATION.GetTextRenderer();
		textRenderer.RequestDrawText(_label, BuiltinResourceKey::DefaultUIFont, screenPosition + Vector2{ 1.f, 1.f }, 20.f, Colors::Black, TextHorizontalAlignment::Center, TextVerticalAlignment::Center);
		textRenderer.RequestDrawText(_label, BuiltinResourceKey::DefaultUIFont, screenPosition, 20.f, Colors::Yellow, TextHorizontalAlignment::Center, TextVerticalAlignment::Center);
	}

	BoundingVolume EnvironmentModelDebugLabelComponent::GetWorldBounds() const
	{
		if (_staticMesh)
			return _staticMesh->GetWorldBounds();
		if (_skeletalMesh)
			return _skeletalMesh->GetWorldBounds();
		return BoundingVolume{};
	}
}
