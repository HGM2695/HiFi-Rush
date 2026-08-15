#pragma once

#include "Component.h"

#include <string>
#include <unordered_map>

namespace gm
{
	class SkeletalMeshComponent;
	class TransformComponent;

	struct Socket
	{
		Vector3			position{};
		Quaternion		rotation{ 0.f, 0.f, 0.f, 1.f };
		Vector3			scale{ 1.f, 1.f, 1.f };
		std::wstring	boneName{};

		Matrix GetLocalMatrix() const;
	};

	class SocketComponent : public Component
	{
	public:
		void			AddSocket(const std::wstring& socketName, const Socket& socket);
		void			RemoveSocket(const std::wstring& socketName);
		void			ClearSockets();

		bool			HasSocket(const std::wstring& socketName) const;
		const Socket*	FindSocket(const std::wstring& socketName) const;
		Matrix			GetSocketWorldMatrix(const std::wstring& socketName) const;

	protected:
		virtual void	OnInitialize() override;

	private:
		std::unordered_map<std::wstring, Socket>	_sockets;
		TransformComponent*							_ownerTransform = nullptr;
		SkeletalMeshComponent*						_skeletalMeshComponent = nullptr;
	};
}
