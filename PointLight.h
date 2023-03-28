#pragma once
#include "RayTracingControl.h"

class PointLight : public GameObject
{
private:

	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;
	BLASBuffers* m_BLASBuffers;
	const aiScene* m_AiScene = NULL;
	ConstantBuffer* m_LightBuffer;
	LightRT LightData;

public:

	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;
};