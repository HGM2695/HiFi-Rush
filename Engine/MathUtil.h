#pragma once

#include "MathTypes.h"
#include <DirectXMath.h>

#include <cmath>

namespace gm::Math
{
	inline constexpr float GM_PI = 3.141592654f;
	inline constexpr float DegreesToRadians(float degrees)
	{
		return degrees * (GM_PI / 180.f);
	}

	inline constexpr float RadiansToDegrees(float radians)
	{
		return radians * (180.f / GM_PI);
	}

	inline float CalcExponentialSmoothingRatio(float interpolationSpeed, float deltaTime)
	{
		return 1.f - std::exp(-interpolationSpeed * deltaTime);
	}

	inline Vector3 EulerDegreesToRadians(const Vector3& eulerDegrees)
	{
		return Vector3(DegreesToRadians(eulerDegrees.x), DegreesToRadians(eulerDegrees.y), DegreesToRadians(eulerDegrees.z));
	}

	inline Matrix IdentityMatrix()
	{
		return Matrix::CreateScale(1.f);
	}

	inline Vector3 GetRightVector(const Quaternion& rotation)
	{
		return Vector3::Transform(Vector3(1.f, 0.f, 0.f), rotation);
	}

	inline Vector3 GetUpVector(const Quaternion& rotation)
	{
		return Vector3::Transform(Vector3(0.f, 1.f, 0.f), rotation);
	}

	inline Vector3 GetLookVector(const Quaternion& rotation)
	{
		return Vector3::Transform(Vector3(0.f, 0.f, 1.f), rotation);
	}

	inline Vector3 ProjectOnXZPlane(const Vector3& vector)
	{
		return Vector3{ vector.x, 0.f, vector.z };
	}

	inline Vector3 GetNormalizedXZDirection(const Vector3& vector)
	{
		Vector3 direction = ProjectOnXZPlane(vector);
		if (direction.LengthSquared() <= 0.000001f)
			return {};

		direction.Normalize();
		return direction;
	}

	inline Quaternion CreateRotationByDirection(const Vector3& direction, float yawOffset = 0.f)
	{
		const float yaw = std::atan2(direction.x, direction.z);
		return Quaternion::CreateFromAxisAngle(Vector3{ 0.f, 1.f, 0.f }, yaw + yawOffset);
	}

	inline Matrix CreateTransformMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale = Vector3(1.f, 1.f, 1.f))
	{
		return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	}

	inline Matrix CreateInverseTransformMatrix(const Vector3& position, const Quaternion& rotation)
	{
		return CreateTransformMatrix(position, rotation).Invert();
	}

	inline Matrix CreateOrthographicLH(float width, float height, float nearZ, float farZ)
	{
		Matrix result;
		DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ));
		return result;
	}

	inline Matrix CreatePerspectiveFieldOfViewLH(float fovYRadians, float aspectRatio, float nearZ, float farZ)
	{
		Matrix result;
		DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixPerspectiveFovLH(fovYRadians, aspectRatio, nearZ, farZ));
		return result;
	}

	inline Matrix CreateLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up)
	{
		Matrix result;
		XMStoreFloat4x4(&result, DirectX::XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up)));
		return result;
	}

	inline bool IntersectsRay(const BoundingOrientedBox& box, const Vector3& origin, const Vector3& direction, float& outDistance)
	{
		return box.Intersects(
			DirectX::XMLoadFloat3(&origin),
			DirectX::XMLoadFloat3(&direction),
			outDistance);
	}

	inline bool IntersectsRay(const BoundingBox& box, const Vector3& origin, const Vector3& direction, float& outDistance)
	{
		return box.Intersects(
			DirectX::XMLoadFloat3(&origin),
			DirectX::XMLoadFloat3(&direction),
			outDistance);
	}

	inline bool IntersectsRay(const BoundingSphere& sphere, const Vector3& origin, const Vector3& direction, float& outDistance)
	{
		return sphere.Intersects(
			DirectX::XMLoadFloat3(&origin),
			DirectX::XMLoadFloat3(&direction),
			outDistance);
	}

	inline Quaternion IdentityQuaternion()
	{
		return Quaternion(0.f, 0.f, 0.f, 1.f);
	}
}
