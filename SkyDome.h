#pragma once

class SkyDome : public GameObject
{
private:

	//DirectX11

	static class Model11* m_Model11;

	//DirectX12

	static class Model12* m_Model12;

	static class RootSignature* m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState12;

public:
	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;
	static void Load(UINT version);
};