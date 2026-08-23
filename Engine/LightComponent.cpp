#include "LightComponent.h"
#include "Application.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "TransformComponent.h"

namespace gm
{
	LightComponent::~LightComponent() = default;

	void LightComponent::SetIntensity(float intensity)
	{
		GM_ASSERT_RETURN(intensity >= 0.f, "Light Intensity는 0 이상이어야 합니다.");
		_intensity = intensity;
	}

	Vector3 LightComponent::GetPosition() const
	{
		GM_ASSERT_RETURN_VAL(_transform, Vector3{}, "Light의 Transform이 준비되지 않았습니다.");
		return _transform->GetPosition();
	}

	Vector3 LightComponent::GetDirection() const
	{
		GM_ASSERT_RETURN_VAL(_transform, Vector3::Forward, "Light의 Transform이 준비되지 않았습니다.");
		return Math::GetLookVector(_transform->GetRotation());
	}

	Vector3 LightComponent::GetRight() const
	{
		GM_ASSERT_RETURN_VAL(_transform, Vector3::Right, "Light의 Transform이 준비되지 않았습니다.");
		return Math::GetRightVector(_transform->GetRotation());
	}

	void LightComponent::OnInitialize()
	{
		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "LightComponent에 필요한 TransformComponent가 없습니다.");
	}

	void LightComponent::OnRender()
	{
		LightRenderItem item{};
		item.lightType = GetLightType();
		item.position = GetPosition();
		item.direction = GetDirection();
		item.color = _color;
		item.intensity = _intensity;
		FillRenderItem(item);
		APPLICATION.GetRenderer().SubmitLight(item);
	}
}
