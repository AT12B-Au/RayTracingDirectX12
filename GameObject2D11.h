#pragma once

class GameObject2D11
{
private:

	VERTEX_3D11* m_Vertex11;

	ComPtr<ID3D11Buffer> m_VertexBuffer11;
	ComPtr<ID3D11ShaderResourceView> m_Texture11;
	ComPtr<ID3D11ShaderResourceView> m_TextureNormal11;
	bool HaveTexture;

	ComPtr<ID3D11VertexShader> m_VertexShader11;
	ComPtr<ID3D11PixelShader> m_PixelShader11;
	ComPtr<ID3D11InputLayout> m_VertexLayout11;

public:
	void Init(VERTEX_3D11* Vertex11,const char* TextureName, const char* VertexShaderName,
		const char* PixelShaderName);
	void InitNormal(const char* TextureName);
	void UnInit();
	void Draw();
	void SetNormal();

	void UpdateVertex(VERTEX_3D11* Vertex11);

	ComPtr<ID3D11Buffer> GetVertexBuffer() { return m_VertexBuffer11; }
	ComPtr<ID3D11ShaderResourceView> GetTexture() { return m_Texture11; }
};