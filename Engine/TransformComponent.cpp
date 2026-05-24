#include "TransformComponent.h"

namespace gm
{
	TransformComponent::TransformComponent() = default;
	TransformComponent::~TransformComponent() = default;

	void TransformComponent::SetRotationEuler(const Vector3& eulerRadians)
	{
		_rotation = Quaternion::CreateFromYawPitchRoll(eulerRadians.y, eulerRadians.x, eulerRadians.z);
	}

	void TransformComponent::SetRotationZ(float radians)
	{
		_rotation = Quaternion::CreateFromAxisAngle(Vector3(0.f, 0.f, 1.f), radians);
	}

	void TransformComponent::SetWorldMatrix(const Matrix& worldMatrix)
	{
		Matrix matrix = worldMatrix;
		matrix.Decompose(_scale, _rotation, _position);
	}

	Matrix TransformComponent::GetWorldMatrix() const
	{
		const Matrix scale = Matrix::CreateScale(_scale);
		const Matrix rotation = Matrix::CreateFromQuaternion(_rotation);
		const Matrix translation = Matrix::CreateTranslation(_position);

		return scale * rotation * translation;
	}
}
