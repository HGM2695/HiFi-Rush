#pragma once

#include "MathTypes.h"

namespace gm::Math
{
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

	inline Matrix CreateWorldMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale = Vector3(1.f, 1.f, 1.f))
	{
		return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	}

	inline Matrix CreateViewMatrix(const Vector3& position, const Quaternion& rotation)
	{
		return CreateWorldMatrix(position, rotation).Invert();
	}
}
