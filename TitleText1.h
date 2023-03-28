#pragma once

class TitleText1 : public GameObject
{
private:

	//DirectX11
	class Model11* m_Model11;

	//‰e—p
	ComPtr<ID3D11VertexShader> m_VertexShaderShadow11 = NULL;
	ComPtr<ID3D11PixelShader> m_PixelShaderShadow11 = NULL;

	//DirectX12

	class Model12* m_Model12;

	class RootSignature* m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState12;

public:


	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;
};