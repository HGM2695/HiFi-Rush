#include "Transform.h"

namespace gm
{
	Transform::Transform() = default;
	Transform::~Transform() = default;

	void Transform::SetRotationEuler(const Vector3& eulerRadians)
	{
		_rotation = Quaternion::CreateFromYawPitchRoll(eulerRadians.y, eulerRadians.x, eulerRadians.z);
	}

	void Transform::SetRotationZ(float radians)
	{
		_rotation = Quaternion::CreateFromAxisAngle(Vector3(0.f, 0.f, 1.f), radians);
	}

	void Transform::SetWorldMatrix(const Matrix& worldMatrix)
	{
		Matrix matrix = worldMatrix;
		matrix.Decompose(_scale, _rotation, _position);
	}

	Matrix Transform::GetWorldMatrix() const
	{
		const Matrix scale = Matrix::CreateScale(_scale);
		const Matrix rotation = Matrix::CreateFromQuaternion(_rotation);
		const Matrix translation = Matrix::CreateTranslation(_position);

		return scale * rotation * translation;
	}
}
