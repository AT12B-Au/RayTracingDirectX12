#pragma once

class StructuredBuffer
{
private:

	ComPtr<ID3D12Resource> m_StructuredBuffer = nullptr;
	ComPtr<ID3D12Resource> m_StructuredBufferUpLoad = nullptr;
	void* m_BuffersOnCPU = nullptr;
	size_t m_NumElement = 0;
	size_t m_SizeOfElement = 0;

public:

	void Init(size_t SizeOfElement, size_t NumElement, void* InitData);
	void Uninit();
	void Update(void* UpdateData);

	void CreateView(D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle);
};