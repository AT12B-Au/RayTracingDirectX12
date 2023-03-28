#pragma once
#include "BLASBuffers.h"

class TLASBuffers
{
private:

	AccelerationStructureBuffers m_TLASBuffers;
	uint64_t m_TLAS_Size;

public:

	void Init(uint64_t ScratchSize, uint64_t ResultSize,int InstanceNum);

	void UnInit()
	{
		if (m_TLASBuffers.scratch != nullptr)
			m_TLASBuffers.scratch->Release();
		if (m_TLASBuffers.result != nullptr)
			m_TLASBuffers.result->Release();
		if (m_TLASBuffers.InstanceDesc != nullptr)
			m_TLASBuffers.InstanceDesc->Release();
	}

	AccelerationStructureBuffers GetTLASBuffers() { return m_TLASBuffers; }
	ComPtr<ID3D12Resource> GetScratchBuffer() { return m_TLASBuffers.scratch; }
	ComPtr<ID3D12Resource> GetResultBuffer() { return m_TLASBuffers.result; }
	ComPtr<ID3D12Resource> GetInstanceDescBuffer() { return m_TLASBuffers.InstanceDesc; }
	uint64_t GetSize() { return m_TLAS_Size; }
};