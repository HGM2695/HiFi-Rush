#include "SocketComponent.h"
#include "GameObject.h"
#include "MathUtil.h"
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

	Matrix SocketComponent::GetSocketWorldMatrix(const std::wstring& socketName) const
	{
		const Socket* socket = FindSocket(socketName);
		if (socket == nullptr)
			return Math::IdentityMatrix();

		return socket->GetLocalMatrix() * _ownerTransform->GetWorldMatrix();
	}

	void SocketComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "SocketComponent 소유자의 Transform이 존재하지 않습니다.");
	}
}
