#pragma once
#include "AppDx11.h"
#include "AppDx12.h"

#define DDS_FORMAT DXGI_FORMAT_BC3_UNORM

class RendererManager
{
public:
	RendererManager() {};
	~RendererManager() {};

	static AppDx11* m_mainDx11;
	static AppDx12* m_mainDx12;

	static void Init(UINT version);
	static void UnInit(UINT version);
	static void Update(UINT version);
	static void Draw(UINT version);
};