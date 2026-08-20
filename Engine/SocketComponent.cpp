#include "SocketComponent.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SkeletalPose.h"
#include "TransformComponent.h"

namespace gm
{
	Matrix Socket::GetLocalMatrix() const
	{
		return Math::CreateTransformMatrix(position, rotation, scale);
	}

	void SocketComponent::AddSocket(const std::wstring& socketName, const Socket& socket)
	{
		GM_ASSERT_RETURN(socketName.empty() == false, "Socket 이름이 비어 있습니다.");
		_sockets[socketName] = socket;
	}

	void SocketComponent::RemoveSocket(const std::wstring& socketName)
	{
		_sockets.erase(socketName);
	}

	void SocketComponent::ClearSockets()
	{
		_sockets.clear();
	}

	bool SocketComponent::HasSocket(const std::wstring& socketName) const
	{
		return _sockets.find(socketName) != _sockets.end();
	}

	const Socket* SocketComponent::FindSocket(const std::wstring& socketName) const
	{
		const auto iter = _sockets.find(socketName);
		if (iter == _sockets.end())
			return nullptr;

		return &iter->second;
	}

	Matrix SocketComponent::GetSocketAnchorWorldMatrix(const std::wstring& socketName) const
	{
		const Socket* socket = FindSocket(socketName);
		if (socket == nullptr)
			return Math::IdentityMatrix();

		Matrix parentMatrix = _ownerTransform->GetWorldMatrix();
		if (socket->boneName.empty() == false && _skeletalMeshComponent != nullptr)
		{
			const std::shared_ptr<SkeletalMesh>& skeletalMesh = _skeletalMeshComponent->GetSkeletalMesh();
			const std::vector<Matrix>& boneModelMatrices = _skeletalMeshComponent->GetPose().GetBoneModelMatrices();
			if (skeletalMesh != nullptr)
			{
				const std::vector<BoneData>& bones = skeletalMesh->GetBones();
				for (uint32 boneIndex = 0; boneIndex < bones.size(); ++boneIndex)
				{
					if (bones[boneIndex].name != socket->boneName || boneIndex >= boneModelMatrices.size())
						continue;

					Matrix boneMatrix = boneModelMatrices[boneIndex] * _skeletalMeshComponent->GetPreTransform();
					Vector3 boneScale{};
					Quaternion boneRotation{};
					Vector3 bonePosition{};
					boneMatrix.Decompose(boneScale, boneRotation, bonePosition);
					boneMatrix = Math::CreateTransformMatrix(bonePosition, boneRotation, Vector3{ 1.f, 1.f, 1.f });
					parentMatrix = boneMatrix * parentMatrix;
					break;
				}
			}
		}

		return parentMatrix;
	}

	Matrix SocketComponent::GetSocketWorldMatrix(const std::wstring& socketName) const
	{
		const Socket* socket = FindSocket(socketName);
		if (socket == nullptr)
			return Math::IdentityMatrix();

		return socket->GetLocalMatrix() * GetSocketAnchorWorldMatrix(socketName);
	}

	void SocketComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "SocketComponent 소유자의 Transform이 존재하지 않습니다.");
		_skeletalMeshComponent = GetOwner().GetComponent<SkeletalMeshComponent>();
	}
}
