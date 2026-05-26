#include "ConstantBufferPool.h"
#include "ConstantBuffer.h"
#include "IGraphicsResourceFactory.h"
#include "GraphicsUtils.h"

namespace gm
{
	ConstantBufferPool::ConstantBufferPool(IGraphicsResourceFactory& resourceFactory)
		: _resourceFactory(resourceFactory)
	{
	}

	ConstantBufferPool::~ConstantBufferPool() = default;

	void ConstantBufferPool::ResetUsage()
	{
		for (auto& [size, bucket] : _buckets)
			bucket.frontIdx = 0;
	}

	ConstantBuffer* ConstantBufferPool::Acquire(uint32 size)
	{
		const uint32 alignedSize = Align16(size);
		GM_ASSERT_RETURN_VAL(alignedSize > 0, nullptr, "ConstantBuffer 크기가 0입니다.");

		BufferBucket& bucket = _buckets[alignedSize];

		if (bucket.frontIdx >= bucket.buffers.size())
			bucket.buffers.push_back(std::unique_ptr<ConstantBuffer>(CreateBuffer(alignedSize)));

		return bucket.buffers[bucket.frontIdx++].get();
	}

	ConstantBuffer* ConstantBufferPool::CreateBuffer(uint32 size)
	{
		ConstantBufferDesc desc{};
		desc.size = size;

		std::unique_ptr<ConstantBuffer> buffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(buffer, nullptr, "ConstantBuffer 생성에 실패했습니다.");

		return buffer.release();
	}
}
