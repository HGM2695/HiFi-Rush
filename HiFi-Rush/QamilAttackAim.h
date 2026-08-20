#pragma once

#include "MathTypes.h"

namespace gm
{
	struct QamilStateContext;

	class QamilAttackAim final
	{
	public:
		void Initialize(const QamilStateContext& context);
		bool ResolveTargetCenter(const QamilStateContext& context, Vector3& outTargetCenter) const;
		bool CalculateTargetOffset(const QamilStateContext& context, const Vector3& attackReferencePosition, float minimumZ, float maximumZ, Vector3& outTargetOffset) const;
		void Apply(QamilStateContext& context, const Vector3& offset);
		void Reset(QamilStateContext& context);

		const Vector3& GetCurrentOffset() const { return _currentOffset; }

	private:
		Matrix _baseMeshPreTransform{};
		Vector3 _currentOffset{};
	};
}
