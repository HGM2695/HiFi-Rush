#include "CascadedShadowMap.h"
#include "CameraViewInfo.h"
#include "IGraphicsResourceFactory.h"
#include "MathUtil.h"
#include "Texture.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace gm
{
	namespace
	{
		using FrustumCorners = std::array<Vector3, 8>;

		Vector3 ClipToWorld(const Vector4& clipPosition, const Matrix& inverseViewProjection)
		{
			const Vector4 worldPosition = Vector4::Transform(clipPosition, inverseViewProjection);
			return Vector3{ worldPosition.x, worldPosition.y, worldPosition.z } / worldPosition.w;
		}

		FrustumCorners CalculateCameraFrustumCorners(const CameraViewInfo& viewInfo, float nearDistance, float farDistance)
		{
			constexpr std::array<Vector2, 4> CornerCoordinates = {
				Vector2{ -1.f, 1.f }, Vector2{ 1.f, 1.f }, Vector2{ 1.f, -1.f }, Vector2{ -1.f, -1.f }
			};
			const Matrix inverseViewProjection = (viewInfo.view * viewInfo.projection).Invert();
			const float cameraDepthRange = viewInfo.farPlane - viewInfo.nearPlane;
			const float nearRatio = (nearDistance - viewInfo.nearPlane) / cameraDepthRange;
			const float farRatio = (farDistance - viewInfo.nearPlane) / cameraDepthRange;
			FrustumCorners corners{};
			for (uint32 cornerIndex = 0; cornerIndex < 4; ++cornerIndex)
			{
				const Vector2 coordinate = CornerCoordinates[cornerIndex];
				const Vector3 cameraNearCorner = ClipToWorld(Vector4{ coordinate.x, coordinate.y, 0.f, 1.f }, inverseViewProjection);
				const Vector3 cameraFarCorner = ClipToWorld(Vector4{ coordinate.x, coordinate.y, 1.f, 1.f }, inverseViewProjection);
				const Vector3 cameraRay = cameraFarCorner - cameraNearCorner;
				corners[cornerIndex] = cameraNearCorner + cameraRay * nearRatio;
				corners[cornerIndex + 4] = cameraNearCorner + cameraRay * farRatio;
			}
			return corners;
		}

		Vector3 CalculateCenter(const FrustumCorners& corners)
		{
			Vector3 center{};
			for (const Vector3& corner : corners)
				center += corner;
			return center / static_cast<float>(corners.size());
		}

		float CalculateStableRadius(const FrustumCorners& corners, const Vector3& center)
		{
			float radius = 0.f;
			for (const Vector3& corner : corners)
				radius = std::max(radius, (corner - center).Length());
			return std::ceil(radius * 16.f) / 16.f;
		}

		Vector3 SnapCenterToShadowTexel(const Vector3& center, const Vector3& lightRight, const Vector3& lightUp, float radius, uint32 mapResolution)
		{
			const float worldUnitsPerTexel = radius * 2.f / static_cast<float>(mapResolution);
			const float centerX = center.Dot(lightRight);
			const float centerY = center.Dot(lightUp);
			const float snappedX = std::round(centerX / worldUnitsPerTexel) * worldUnitsPerTexel;
			const float snappedY = std::round(centerY / worldUnitsPerTexel) * worldUnitsPerTexel;
			return center + lightRight * (snappedX - centerX) + lightUp * (snappedY - centerY);
		}
	}

	bool CascadedShadowMap::Initialize(IGraphicsResourceFactory& resourceFactory, const ShadowSettings& settings)
	{
		GM_ASSERT_RETURN_VAL(settings.mapResolution > 0, false, "Shadow Map Resolution은 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(settings.cascadeCount > 0 && settings.cascadeCount <= MaxShadowCascadeCount, false, "Shadow Cascade Count가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(settings.maxDistance > 0.f, false, "Shadow Distance는 0보다 커야 합니다.");
		GM_ASSERT_RETURN_VAL(settings.cascadeSplitLambda >= 0.f && settings.cascadeSplitLambda <= 1.f, false, "Cascade Split Lambda는 0과 1 사이여야 합니다.");
		GM_ASSERT_RETURN_VAL(settings.depthBias >= 0.f && settings.normalBias >= 0.f, false, "Shadow Bias는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(settings.shadowStrength >= 0.f && settings.shadowStrength <= 1.f, false, "Shadow Strength는 0과 1 사이여야 합니다.");

		TextureDesc desc{};
		desc.width = settings.mapResolution;
		desc.height = settings.mapResolution;
		desc.arraySize = MaxShadowCascadeCount;
		desc.format = TextureFormat::Depth32Float;
		desc.bindUsage = TextureBindUsage::DepthStencil | TextureBindUsage::ShaderResource;
		_texture = resourceFactory.CreateTexture(desc);
		GM_ASSERT_RETURN_VAL(_texture, false, "Cascaded Shadow Map 생성에 실패했습니다.");
		_settings = settings;
		return true;
	}

	void CascadedShadowMap::UpdateSettings(const ShadowSettings& settings)
	{
		_settings = settings;
	}

	void CascadedShadowMap::UpdateCascades(const CameraViewInfo& viewInfo, const Vector3& lightDirection)
	{
		Vector3 lightForward = lightDirection;
		lightForward.Normalize();

		const float nearDistance = viewInfo.nearPlane;
		const float farDistance = std::min(viewInfo.farPlane, _settings.maxDistance);
		const float depthRange = farDistance - nearDistance;
		const float depthRatio = farDistance / nearDistance;
		std::array<float, MaxShadowCascadeCount> splitDistances{};
		for (uint32 cascadeIndex = 0; cascadeIndex < _settings.cascadeCount; ++cascadeIndex)
		{
			const float splitRatio = static_cast<float>(cascadeIndex + 1) / static_cast<float>(_settings.cascadeCount);
			const float logarithmicSplit = nearDistance * std::pow(depthRatio, splitRatio);
			const float uniformSplit = nearDistance + depthRange * splitRatio;
			splitDistances[cascadeIndex] = std::lerp(uniformSplit, logarithmicSplit, _settings.cascadeSplitLambda);
		}

		const Vector3 referenceUp = Vector3::Up;
		Vector3 lightRight = referenceUp.Cross(lightForward);
		lightRight.Normalize();
		Vector3 lightUp = lightForward.Cross(lightRight);
		lightUp.Normalize();
		float cascadeNearDistance = nearDistance;
		for (uint32 cascadeIndex = 0; cascadeIndex < _settings.cascadeCount; ++cascadeIndex)
		{
			const float cascadeFarDistance = splitDistances[cascadeIndex];
			const FrustumCorners corners = CalculateCameraFrustumCorners(viewInfo, cascadeNearDistance, cascadeFarDistance);
			Vector3 cascadeCenter = CalculateCenter(corners);
			const float cascadeRadius = CalculateStableRadius(corners, cascadeCenter);
			cascadeCenter = SnapCenterToShadowTexel(cascadeCenter, lightRight, lightUp, cascadeRadius, _settings.mapResolution);

			const float casterDepth = _settings.maxDistance + cascadeRadius;
			const Vector3 lightPosition = cascadeCenter - lightForward * casterDepth;
			const Matrix lightView = Math::CreateLookAtLH(lightPosition, cascadeCenter, lightUp);
			const Matrix lightProjection = Math::CreateOrthographicLH(cascadeRadius * 2.f, cascadeRadius * 2.f, 0.f, casterDepth * 2.f);
			ShadowCascade& cascade = _cascades[cascadeIndex];
			cascade.viewProjection = lightView * lightProjection;
			cascade.nearDistance = cascadeNearDistance;
			cascade.farDistance = cascadeFarDistance;
			const BoundingOrientedBox lightSpaceCasterBounds{ Vector3{ 0.f, 0.f, casterDepth }, Vector3{ cascadeRadius, cascadeRadius, casterDepth }, Quaternion::Identity };
			lightSpaceCasterBounds.Transform(cascade.casterBounds, lightView.Invert());
			_renderData.viewProjections[cascadeIndex] = cascade.viewProjection;
			cascadeNearDistance = cascadeFarDistance;
		}

		_renderData.splitDistances = Vector4{ splitDistances[0], splitDistances[1], splitDistances[2], splitDistances[3] };
		_renderData.cascadeCount = _settings.cascadeCount;
		_renderData.pcfRadius = _settings.pcfRadius;
		_renderData.depthBias = _settings.depthBias;
		_renderData.normalBias = _settings.normalBias;
		_renderData.shadowMapTexelSize = 1.f / static_cast<float>(_settings.mapResolution);
		_renderData.shadowStrength = _settings.shadowStrength;
		const Color linearShadowColor = ConvertSRGBToLinear(_settings.shadowColor);
		_renderData.shadowColor = Vector3{ linearShadowColor.x, linearShadowColor.y, linearShadowColor.z };
	}

	void CascadedShadowMap::ClearCascades()
	{
		_renderData = CascadedShadowRenderData{};
	}
}
