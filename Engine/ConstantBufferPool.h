#pragma once

#include "Types.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace gm
{
	class ConstantBuffer;
	class IGraphicsResourceFactory;

	class ConstantBufferPool
	{
	public:
		explicit ConstantBufferPool(IGraphicsResourceFactory& resourceFactory);
		~ConstantBufferPool();

		void			ResetUsage();
		ConstantBuffer*	Acquire(uint32 size);

	private:
		struct BufferBucket
		{
			std::vector<std::unique_ptr<ConstantBuffer>>	buffers;
			uint32											frontIdx = 0;
		};

		ConstantBuffer*	CreateBuffer(uint32 size);

	private:
		IGraphicsResourceFactory&					_resourceFactory;
		std::unordered_map<uint32, BufferBucket>	_buckets;
	};
}
