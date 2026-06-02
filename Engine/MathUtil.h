#pragma once

#include "MathTypes.h"
#include <DirectXMath.h>

namespace gm::Math
{
	inline constexpr float GM_PI = 3.141592654f;

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

	inline Quaternion IdentityQuaternion()
	{
		return Quaternion(0.f, 0.f, 0.f, 1.f);
	}
}
