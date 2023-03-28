#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "BLASBuffers.h"
#include "TLASBuffers.h"
#include "DxilLibrary.h"
#include "RootSignature.h"
#include "StructuredBuffer.h"
#include "ShaderTable.h"
#include "ConstantBuffer.h"
#include "Texture.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

static const WCHAR* RayGenShaderName = L"RayGen";
static const WCHAR* MissShaderName[] = 
{
	L"Miss",
	L"ShadowMiss",
};
static const WCHAR* ClosestHitShaderName[] =
{
	L"ClosestHit",
	L"BoxHit",
	L"FloorHit",
	L"PointLightHit",
	L"PillarHit",
	L"CactiHit",
};
static const WCHAR* HitGroupName[] = 
{
	L"BallHitGroup",
	L"BoxHitGroup",
	L"FloorHitGroup",
	L"PointLightHitGroup",
	L"PillarHitGroup",
	L"CactiHitGroup",
};

static const char* ShaderName[] =
{
	"shader\\RayTracing.hlsl",
	"shader\\BoxRayTracing.hlsl",
	"shader\\FloorRayTracing.hlsl",
	"shader\\PointLightRayTracing.hlsl",
	"shader\\PillarRayTracing.hlsl",
	"shader\\CactiRayTracing.hlsl",
};

#define AllocateDescriptorsNum 100

struct PayLoad
{
	XMFLOAT3 color;
	bool hit; // 衝突フラグ
	int reflection; // リフレクションフラグ
};

struct DescriptorTable
{
	D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
	UINT NumDescriptors;
	UINT BaseShaderRegister;
	UINT RegisterSpace;
	D3D12_DESCRIPTOR_RANGE_FLAGS flags;
	int TableNum;
};

struct CameraRT
{
	XMMATRIX CameraRot;
	XMFLOAT3 pos;
	float aspect;
	float f_near;
	float f_far;
};

struct LightRT
{
	XMFLOAT3 direction;			//平行光源のみ
	XMFLOAT4 color;
	XMFLOAT4 AmbientColor;
	XMFLOAT3 PointLightPos;
	UINT ShadowRayCount;
	bool flag;				   //平行光源か点光源か
};

class RayTracingControl
{
public:

	static TLASBuffers* m_TLASBuffers;
	static ComPtr<ID3D12Resource> m_OutputBuffer;
	static ConstantBuffer* m_CameraBuffer;
	static CameraRT CameraData;
	static ShaderTable* m_ShaderTable;
	static ComPtr<ID3D12RootSignature> m_RayGenLocal;
	static ComPtr<ID3D12StateObject> m_PSO;
	static DxilLibrary** m_DxilLibrary;
	static RootSignature* m_GlobalRootSignature;
	static boost::container::vector<DescriptorTable> m_DescriptorTableData;
	static UINT BaseShaderRegisterUAV;
	static UINT BaseShaderRegisterSRV;
	static UINT BaseShaderRegisterCBV;
	static int TotalTableNum;
	static int* TableStart;
	static ComPtr<ID3D12DescriptorHeap> m_DescHeap;
	static int m_DescriptorNum;
	static Texture* m_BackgroundTexture;

	static void InitDescHeap();

	static void Init();
	static void UnInit();
	static void Update(); 
	static void Begin();
	static void End();
};