#pragma once

class Player : public GameObject
{
private:

	//DirectX11

	static class AnimationModel11* m_Model11;

	D3DXQUATERNION m_Quaternion11;
	D3DXVECTOR3 m_ObjectRotation11;

	int m_Frame11 = 0;
	float m_BlendRate11;

	std::string m_AnimationNormal11;
	std::string m_AnimationAction11;

	//影用
	ID3D11VertexShader* m_VertexShaderShadow11 = NULL;
	ID3D11PixelShader* m_PixelShaderShadow11 = NULL;

	//DirectX12

	static class AnimationModel12* m_Model12;

	static class RootSignature* m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState12;

	//影用
	//static class RootSignature* m_RootSignatureShadow;
	ComPtr<ID3DBlob> m_VertexShaderShadow12;
	ComPtr<ID3DBlob> m_PixelShaderShadow12;
	ComPtr<ID3D12PipelineState> m_PipelineStateShadow12;

	std::string m_AnimationNormal12;
	std::string m_AnimationAction12;

	//XMVECTOR m_Quaternion12;
	XMFLOAT3 m_ObjectRotation12;

	int m_Frame12 = 0;
	float m_BlendRate12;

	//共有
 
	//コントローラー
	//std::unique_ptr<GamePad> m_GamePad;

	//カメラスピード
	float m_CameraSpeed;

	//アニメションスピード
	float m_AnimationSpeed;
public:


	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;
	static void Load(UINT version);
};