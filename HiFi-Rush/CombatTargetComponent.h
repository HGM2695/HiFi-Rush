#pragma once

#include "Component.h"

#include <string>
#include <vector>

namespace gm
{
	class HealthComponent;
	class SocketComponent;
	class TransformComponent;

	class CombatTargetComponent final : public Component
	{
	public:
		CombatTargetComponent() = default;
		explicit CombatTargetComponent(const Vector3& localTargetOffset);

		bool	AddLocalTargetPoint(const std::wstring& targetPointId, const Vector3& localOffset = {});
		bool	AddSocketTargetPoint(const std::wstring& targetPointId, const std::wstring& socketName);
		bool	SetTargetPointEnabled(const std::wstring& targetPointId, bool isEnabled);
		bool	IsTargetable() const;
		bool	IsTargetPointTargetable(uint32 targetPointIndex) const;
		uint32	GetTargetPointCount() const { return static_cast<uint32>(_targetPoints.size()); }
		Vector3 GetTargetPosition(uint32 targetPointIndex) const;

	protected:
		void OnInitialize() override;

	private:
		struct TargetPoint
		{
			std::wstring id{};
			std::wstring socketName{};
			Vector3 localOffset{};
			bool isEnabled = true;
		};

		bool HasTargetPoint(const std::wstring& targetPointId) const;

		TransformComponent*		_ownerTransform = nullptr;
		HealthComponent*		_healthComponent = nullptr;
		SocketComponent*		_socketComponent = nullptr;
		std::vector<TargetPoint>	_targetPoints{};
	};
}
