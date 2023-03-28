#pragma once
#include "RayTracingControl.h"

#define FLOOR_LENGTH (9)		//äÔêîÇÃÇ›
#define FLOOR_NUM (90)
#define FLOOR_INTERVAL (2.0f)

class Floor : public GameObject
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