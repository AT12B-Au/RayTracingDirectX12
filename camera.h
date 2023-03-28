#pragma once

class Camera : public GameObject
{
private:

	//DirectX11
	D3DXVECTOR3 m_Target11;
	D3DXMATRIX m_ViewMatrix11;

	//DirectX12
	XMFLOAT3 m_Target12;
	XMMATRIX m_ViewMatrix12;
	XMMATRIX m_ProjMatrix12;

public:

	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;

	D3DXMATRIX GetViewMatrix11() { return m_ViewMatrix11; }
	XMMATRIX GetViewMatrix12() { return m_ViewMatrix12; }
	XMMATRIX GetProjMatrix12() { return m_ProjMatrix12; }
};