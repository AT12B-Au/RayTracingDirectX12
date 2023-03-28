#pragma once
#include "RayTracingControl.h"

class Ball : public GameObject
{
private:

	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;
	StructuredBuffer* m_StructuredVertexBuffer;
	StructuredBuffer* m_StructuredIndexBuffer;
	BLASBuffers* m_BLASBuffers;
	const aiScene* m_AiScene = NULL;
	Texture* m_Texture;

public:

	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;
};