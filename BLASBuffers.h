#pragma once

struct AccelerationStructureBuffers
{
	ComPtr<ID3D12Resource> scratch = nullptr;
	ComPtr<ID3D12Resource> result = nullptr;
	ComPtr<ID3D12Resource> InstanceDesc = nullptr;
};

class BLASBuffers
{
private:

	AccelerationStructureBuffers m_BLBuffers;

public:

	void Init(uint64_t ScratchSize, uint64_t ResultSize);

	void UnInit()
	{
		if (m_BLBuffers.scratch != nullptr)
			m_BLBuffers.scratch->Release();
		if (m_BLBuffers.result != nullptr)
			m_BLBuffers.result->Release();
		if (m_BLBuffers.InstanceDesc != nullptr)
			m_BLBuffers.InstanceDesc->Release();
	}

	AccelerationStructureBuffers GetBLBuffers() { return m_BLBuffers; }
	ComPtr<ID3D12Resource> GetScratchBuffer() { return m_BLBuffers.scratch; }
	ComPtr<ID3D12Resource> GetResultBuffer() { return m_BLBuffers.result; }
};