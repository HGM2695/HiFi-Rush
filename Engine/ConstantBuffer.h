#pragma once

#include "EngineCore.h"

namespace gm
{
	struct ConstantBufferDesc
	{
		uint32		size = 0;
		const void* initialData = nullptr;
	};

	class ConstantBuffer
	{
	public:
		virtual ~ConstantBuffer() = default;

		uint32 GetSize() const { return _size; }

	protected:
		ConstantBuffer(uint32 size) : _size(size) {};

	private:
		uint32 _size = 0;
	};
}