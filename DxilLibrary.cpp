#include "AppDx12.h"
#include "DxilLibrary.h"
#include "RendererManager.h"
#include <atlbase.h>
#include <fstream>
#include <sstream>

void DxilLibrary::Init(const std::string FileName,LPCWSTR Target)
{
	HRESULT hr;

	std::ifstream FileStream(FileName);

	if (!FileStream.is_open())
		throw std::runtime_error("OpenShaderFileFailed");

	//HLSLファイルを読み込む
	std::stringstream buffer;
	buffer << FileStream.rdbuf();
	std::string code = buffer.str();

	//コンパイル設定を作成する
	ComPtr<IDxcCompiler> DxcCompiler;
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcCompiler));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIDxcCompilerFailed");
	}

	ComPtr<IDxcLibrary> DxcLib;
	hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&DxcLib));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIDxcLibraryFailed");
	}

	ComPtr<IDxcBlobEncoding> SourceBlob;
	hr = DxcLib->CreateBlobWithEncodingFromPinned(code.c_str(),
		static_cast<UINT32>(code.length()), CP_UTF8, &SourceBlob);
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIDxcBlobEncodingFailed");
	}

	std::wstring WideFilePath(FileName.begin(), FileName.end());
	LPCWSTR LpcFilePath = WideFilePath.c_str();

	ComPtr<IDxcIncludeHandler> IncluderHandler;
	DxcLib->CreateIncludeHandler(&IncluderHandler);

	ComPtr<IDxcBlob> IncludeSource;
	IncluderHandler->LoadSource(L"shader\\RayTracingHeader.hlsli", &IncludeSource);

	ComPtr<IDxcOperationResult> result;
	hr = DxcCompiler->Compile(SourceBlob.Get(), LpcFilePath, L"", Target, nullptr, 0, nullptr, 0, IncluderHandler.Get(), &result);
	if (FAILED(hr))
	{
		throw std::runtime_error("ShaderCompileFailed");
	}

	
	hr = result->GetResult(&m_DxcBlob);
	if (FAILED(hr))
	{
		throw std::runtime_error("GetCompileResultFailed");
	}

	IncluderHandler->Release();
}

D3D12_STATE_SUBOBJECT DxilLibrary::CreateDxilLibrary(const WCHAR* EntryPoint[], int EntriesNum)
{
	D3D12_DXIL_LIBRARY_DESC* desc = new D3D12_DXIL_LIBRARY_DESC;
	D3D12_STATE_SUBOBJECT SubObject = {};
	boost::container::vector<D3D12_EXPORT_DESC>* ExportDesc = new boost::container::vector<D3D12_EXPORT_DESC>(EntriesNum);
	boost::container::vector<std::wstring>* ExportName = new boost::container::vector<std::wstring>(EntriesNum);

	ZeroMemory(desc, sizeof(D3D12_DXIL_LIBRARY_DESC));
	ExportDesc->resize(EntriesNum);
	ExportName->resize(EntriesNum);

	if (m_DxcBlob)
	{
#pragma omp parallel for
		for (int i = 0; i < EntriesNum; i++)
		{
			ExportName->at(i) = EntryPoint[i];
			ExportDesc->at(i).Name = ExportName->at(i).c_str();
			ExportDesc->at(i).Flags = D3D12_EXPORT_FLAG_NONE;
			ExportDesc->at(i).ExportToRename = nullptr;
		}

		desc->DXILLibrary.pShaderBytecode = m_DxcBlob->GetBufferPointer();
		desc->DXILLibrary.BytecodeLength = m_DxcBlob->GetBufferSize();
		desc->NumExports = EntriesNum;
		desc->pExports = ExportDesc->data();

		SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		SubObject.pDesc = desc;
	}

	return SubObject;
}