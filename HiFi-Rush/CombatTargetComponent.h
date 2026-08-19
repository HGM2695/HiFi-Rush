#pragma once

#include "Component.h"

namespace gm
{
	class HealthComponent;
	class TransformComponent;

	class CombatTargetComponent final : public Component
	{
	public:
		explicit CombatTargetComponent(const Vector3& localTargetOffset = {});

		bool			IsTargetable() const;
		Vector3			GetTargetPosition() const;
		void			SetLocalTargetOffset(const Vector3& offset) { _localTargetOffset = offset; }
		const Vector3&	GetLocalTargetOffset() const { return _localTargetOffset; }

	protected:
		void			OnInitialize() override;

	private:
		TransformComponent*		_ownerTransform = nullptr;
		HealthComponent*		_healthComponent = nullptr;
		Vector3					_localTargetOffset{};
	};
}
