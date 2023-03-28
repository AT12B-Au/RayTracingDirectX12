#pragma once
#include <dxgi.h>
#include <dxcapi.h>

#pragma comment(lib,"dxcompiler.lib")

class DxilLibrary
{
private:

	ComPtr<IDxcBlob> m_DxcBlob;

public:

	void Init(const std::string FileName, LPCWSTR Target);
	void UnInit()
	{
		if (m_DxcBlob != nullptr)
			m_DxcBlob->Release();
	}
	ComPtr<IDxcBlob> GetDxcBlob() { return m_DxcBlob; }
	D3D12_STATE_SUBOBJECT CreateDxilLibrary(const WCHAR* EntryPoint[], int EntriesNum);
};