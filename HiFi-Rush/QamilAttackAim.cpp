#include "QamilAttackAim.h"

#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HurtBoxComponent.h"
#include "MathUtil.h"
#include "QamilStateContext.h"
#include "SkeletalMeshComponent.h"
#include "TransformComponent.h"

#include <algorithm>

namespace gm
{
	void QamilAttackAim::Initialize(const QamilStateContext& context)
	{
		_baseMeshPreTransform = context.skeletalMeshComponent->GetPreTransform();
		_currentOffset = {};
	}

	bool QamilAttackAim::ResolveTargetCenter(const QamilStateContext& context, Vector3& outTargetCenter) const
	{
		const GameObject* target = context.target.Get();
		if (target == nullptr || target->GetTransform() == nullptr)
			return false;

		const HurtBoxComponent* hurtBox = target->GetComponent<HurtBoxComponent>();
		const Vector3 localTargetCenter = hurtBox ? hurtBox->GetCollider().GetLocalCenter() : Vector3{};
		outTargetCenter = Vector3::Transform(localTargetCenter, target->GetTransform()->GetWorldMatrix());
		return true;
	}

	bool QamilAttackAim::CalculateTargetOffset(const QamilStateContext& context, const Vector3& attackReferencePosition, float minimumZ, float maximumZ, Vector3& outTargetOffset) const
	{
		Vector3 targetCenter{};
		if (ResolveTargetCenter(context, targetCenter) == false || context.transformComponent == nullptr)
			return false;
		const Vector3 targetLocalPosition = Vector3::Transform(targetCenter, context.transformComponent->GetWorldMatrix().Invert());
		outTargetOffset = Math::ProjectOnXZPlane(targetLocalPosition - attackReferencePosition);
		outTargetOffset.z = std::clamp(outTargetOffset.z, minimumZ, maximumZ);
		return true;
	}

	void QamilAttackAim::Apply(QamilStateContext& context, const Vector3& offset)
	{
		_currentOffset = offset;
		context.skeletalMeshComponent->SetPreTransform(_baseMeshPreTransform * Matrix::CreateTranslation(offset));
	}

	void QamilAttackAim::Reset(QamilStateContext& context)
	{
		_currentOffset = {};
		context.skeletalMeshComponent->SetPreTransform(_baseMeshPreTransform);
	}
}
