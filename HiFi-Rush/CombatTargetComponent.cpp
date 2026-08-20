#include "CombatTargetComponent.h"

#include "GameObject.h"
#include "HealthComponent.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

namespace gm
{
	CombatTargetComponent::CombatTargetComponent(const Vector3& localTargetOffset)
	{
		AddLocalTargetPoint(L"Body", localTargetOffset);
	}

	bool CombatTargetComponent::AddLocalTargetPoint(const std::wstring& targetPointId, const Vector3& localOffset)
	{
		GM_ASSERT_RETURN_VAL(targetPointId.empty() == false, false, "Combat Target Point ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(HasTargetPoint(targetPointId) == false, false, "Combat Target Point ID가 중복되었습니다. id=%ls", targetPointId.c_str());
		_targetPoints.push_back(TargetPoint{ .id = targetPointId, .localOffset = localOffset });
		return true;
	}

	bool CombatTargetComponent::AddSocketTargetPoint(const std::wstring& targetPointId, const std::wstring& socketName)
	{
		GM_ASSERT_RETURN_VAL(targetPointId.empty() == false, false, "Combat Target Point ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(socketName.empty() == false, false, "Combat Target Point Socket 이름은 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(HasTargetPoint(targetPointId) == false, false, "Combat Target Point ID가 중복되었습니다. id=%ls", targetPointId.c_str());
		_targetPoints.push_back(TargetPoint{ .id = targetPointId, .socketName = socketName });
		return true;
	}

	bool CombatTargetComponent::SetTargetPointEnabled(const std::wstring& targetPointId, bool isEnabled)
	{
		for (TargetPoint& targetPoint : _targetPoints)
		{
			if (targetPoint.id != targetPointId)
				continue;
			targetPoint.isEnabled = isEnabled;
			return true;
		}
		return false;
	}

	void CombatTargetComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "CombatTargetComponent는 TransformComponent가 필요합니다.");

		_healthComponent = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(_healthComponent, "CombatTargetComponent는 HealthComponent가 필요합니다.");
		GM_ASSERT_RETURN(_targetPoints.empty() == false, "CombatTargetComponent에는 하나 이상의 Target Point가 필요합니다.");
		_socketComponent = GetOwner().GetComponent<SocketComponent>();
		for (const TargetPoint& targetPoint : _targetPoints)
		{
			if (targetPoint.socketName.empty())
				continue;
			GM_ASSERT_RETURN(_socketComponent && _socketComponent->HasSocket(targetPoint.socketName), "Combat Target Point가 참조하는 Socket을 찾을 수 없습니다. point=%ls, socket=%ls", targetPoint.id.c_str(), targetPoint.socketName.c_str());
		}
	}

	bool CombatTargetComponent::IsTargetable() const
	{
		return IsEnabled() && _healthComponent && _healthComponent->IsEnabled() && _healthComponent->IsDead() == false && GetOwner().IsPendingDestroy() == false;
	}

	bool CombatTargetComponent::IsTargetPointTargetable(uint32 targetPointIndex) const
	{
		return IsTargetable() && targetPointIndex < _targetPoints.size() && _targetPoints[targetPointIndex].isEnabled;
	}

	Vector3 CombatTargetComponent::GetTargetPosition(uint32 targetPointIndex) const
	{
		if (_ownerTransform == nullptr || targetPointIndex >= _targetPoints.size())
			return {};
		const TargetPoint& targetPoint = _targetPoints[targetPointIndex];
		if (targetPoint.socketName.empty() == false)
		{
			if (_socketComponent == nullptr || _socketComponent->HasSocket(targetPoint.socketName) == false)
				return {};
			return Vector3::Transform(Vector3{}, _socketComponent->GetSocketWorldMatrix(targetPoint.socketName));
		}
		return Vector3::Transform(targetPoint.localOffset, _ownerTransform->GetWorldMatrix());
	}

	bool CombatTargetComponent::HasTargetPoint(const std::wstring& targetPointId) const
	{
		for (const TargetPoint& targetPoint : _targetPoints)
		{
			if (targetPoint.id == targetPointId)
				return true;
		}
		return false;
	}
}
