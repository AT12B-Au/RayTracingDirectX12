#pragma once

class Field : public GameObject
{

private:
	D3DXVECTOR3	m_Position;
	D3DXVECTOR3	m_Rotation;
	D3DXVECTOR3	m_Scale;

	ID3D11Buffer* m_VertexBuffer = NULL;
	ID3D11ShaderResourceView* m_Texture = NULL;

	ID3D11VertexShader* m_VertexShader; //���_�V�F�[�_�[�I�u�W�F�N�g
	ID3D11PixelShader* m_PixelShader; //�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ID3D11InputLayout* m_VertexLayout; //���_���C�A�E�g�I�u�W�F�N�g

	ID3D11ShaderResourceView* m_TextureNormal = NULL;

public:
	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;

};