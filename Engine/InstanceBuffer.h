#pragma once

#include "EngineCore.h"

namespace gm
{
	struct InstanceBufferDesc
	{
		uint32 stride = 0;
		uint32 capacity = 0;
	};

	class InstanceBuffer
	{
	public:
		virtual ~InstanceBuffer() = default;

		uint32 GetStride() const { return _stride; }
		uint32 GetCapacity() const { return _capacity; }

	protected:
		InstanceBuffer(uint32 stride, uint32 capacity) : _stride(stride), _capacity(capacity) {}

	private:
		uint32 _stride = 0;
		uint32 _capacity = 0;
	};
}
