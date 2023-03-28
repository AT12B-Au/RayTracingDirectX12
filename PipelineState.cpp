#include "AppDx12.h"
#include "PipelineState.h"
#include "RendererManager.h"

void PipelineState::Init(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
{
	auto m_Device = GetDX12Renderer->GetDevice();

	HRESULT hr = m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_PipelineState));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreatePipelineStateFailed");
	}

	m_PipelineState->SetName(L"PipelineState");
}