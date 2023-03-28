#pragma once
#include "AnimationModel11.h"

class Model11
{
private:
	const aiScene* m_AiScene = NULL;

	//DirectX11
	ID3D11Buffer** m_VertexBuffer11;
	ID3D11Buffer** m_IndexBuffer11;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Texture11;
	ID3D11ShaderResourceView* m_TextureNormal11 = NULL;

public:
	void Load(const char* FileName);
	void InitNormal(const char* TextureName);
	void UnLoad();
	void Draw();
};