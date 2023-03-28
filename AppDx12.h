//�K�v�ȃw�b�_�[�t�@�C���̃C���N���[�h
#pragma once
#define NOMINMAX
#include <windows.h>
#include <initguid.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "WICTextureLoader12.h"
#include "DDSTextureLoader12.h"
#include <wrl.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include <stdexcept>
#include <boost/container/vector.hpp>

//�K�v�ȃ��C�u�����t�@�C���̃��[�h
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"winmm.lib") 
//#pragma comment(lib,"DirectXTK12.lib")
#pragma comment(lib,"dxgi.lib")


//�l�[���X�y�[�X
using namespace DirectX;
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

//�萔��`
#define FRAME_COUNT (3)//��ʃo�b�t�@��
#define GetDX12Renderer	RendererManager::m_mainDx12

#define DESCHEAP_NUM (5+1)
#define TEXTURE_LEVEL (5)

#define AlignTo(alignment, val) (((val + alignment - 1) / alignment) * alignment)

//���_�\���� ��`

//�R���X�^���g�o�b�t�@
struct LIGHT12
{
	BOOL		Enable;
	BOOL		Dummy[3];
	XMVECTOR	Direction;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Ambient;
	XMMATRIX	ViewMatrix;			//���C�g�J�����s��
	XMMATRIX	ProjectionMatrix;	//���C�g�v���W�F�N�V�����s��
};

struct MATERIAL12
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	float		Dummy[3];
};

struct GlobalBuffer12
{
	//XMMATRIX Wvp;//���[���h����ˉe�܂ł̕ϊ��s��
	XMFLOAT4 g_CameraPosition;//���_�ʒu
	XMFLOAT4 g_Parameter;
	MATERIAL12 material;
};

struct VERTEX_3D12
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};

struct VERTEX_RT_3D12
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};

enum
{
	BALL_HIT_GROUP,
	BOX_HIT_GROUP,
	FLOOR_HIT_GROUP,
	POINT_LIGHT_HIT_GROUP,
	PILLAR_HIT_GROUP,
	CACTI_HIT_GROUP,
};

//
//
//MAIN�N���X
class AppDx12
{
private:
	UINT button;

	ComPtr<ID3D12Device8> m_Device = NULL;
	
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator = NULL;
	ComPtr<ID3D12CommandQueue> m_CommandQueue = NULL;
	ComPtr<ID3D12GraphicsCommandList4> m_CommandList = NULL;
	ComPtr<IDXGISwapChain3> m_SwapChain = NULL;
	ComPtr<ID3D12Fence> m_Fence = NULL;
	UINT64 m_FenceValue;

	UINT m_RtvStride;
	UINT m_DsvStride;
	UINT m_DbvSrvStride;
	UINT m_DBSize;

	//RTV, DSV�n
	ComPtr<ID3D12Resource> m_RenderTargets[FRAME_COUNT];
	ComPtr<ID3D12Resource> m_DepthBuffer = NULL;
	ComPtr<ID3D12DescriptorHeap> m_RtvHeap = NULL;
	ComPtr<ID3D12DescriptorHeap> m_DspHeap = NULL;

	//�f�o�b�N
	ComPtr<ID3D12Debug> DebugController12 = NULL;

	//Imgui
	ComPtr<ID3D12DescriptorHeap> m_ImGuiHeap = NULL;

	D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_WBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_VBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_PBVDesc12;

	class RootSignature* m_RootSignature = NULL;

	//�V���h�E
	ComPtr<ID3D12DescriptorHeap> m_ShadowDescHeap;	//�V���h�E�}�b�v�p�[�x�o�b�t�@�p�f�X�N���v�^�q�[�v
	//ComPtr<ID3D12DescriptorHeap> m_ShadowTextureDescHeap; //�V���h�E�}�b�v�p�[�x�e�N�X�`���p�f�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource>		m_ShadowBuffer;		//�V���h�E�}�b�v�p�[�x�o�b�t�@
	//ComPtr<ID3D12PipelineState>	m_ShadowPipelineState;	//�V���h�E�}�b�v�p�̃p�C�v���C��

public:
	HRESULT Init();
	void Update();
	void Begin();
	void End();
	void WaitGpu();
	void UnInit();
	UINT GetButton() { return button; }

	ComPtr<ID3D12Resource> CreateVertexBuffer(UINT m_VertexBufferSize);
	ComPtr<ID3D12Resource> CreateVertexBufferUpLoad(UINT m_VertexBufferSize);
	D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(ComPtr<ID3D12Resource> m_VertexBuffer, UINT m_VertexBufferSize);
	ComPtr<ID3D12Resource> CreateTexture(DXGI_FORMAT format, UINT width, UINT height);
	ComPtr<ID3D12Resource> CreateTextureUpLoad(ComPtr<ID3D12Resource> m_Texture);
	void CreateTextureView(ComPtr<ID3D12Resource> m_Texture, ComPtr<ID3D12DescriptorHeap> m_DescHeap, DXGI_FORMAT Format, UINT NumDescriptors);
	void CreateShadowView(ComPtr<ID3D12DescriptorHeap> m_DescHeap, UINT NumDescriptors);

	ComPtr<ID3D12Resource> CreateConstantBuffer(UINT m_CBSize);
	ComPtr<ID3D12DescriptorHeap> CreateConstantBufferViewHeap(UINT num);
	D3D12_CONSTANT_BUFFER_VIEW_DESC CreateConstantBufferView(ComPtr<ID3D12Resource> m_ConstantBuffer,ComPtr<ID3D12DescriptorHeap> m_DescHeap,UINT num, UINT CBSize);
	ComPtr<ID3D12Resource> CreateIndexBuffer(UINT m_IndexBufferSize);
	ComPtr<ID3D12Resource> CreateIndexBufferUpLoad(UINT m_IndexBufferSize);
	D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(ComPtr<ID3D12Resource> m_IndexBuffer, UINT m_IndexBufferSize);
	ComPtr<ID3D12PipelineState> CreatePipelineState(ComPtr<ID3DBlob> m_VertexShader, ComPtr<ID3DBlob> m_PixelShader, ComPtr<ID3D12RootSignature> m_RootSignature);
	void CreateOutputBarrier(ComPtr<ID3D12Resource> OutputBuffer);
	void CopyResource(ComPtr<ID3D12Resource> resource);
	void ChangeTransition(ComPtr<ID3D12Resource> resource,D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	UINT GetBackBufferIndex();

	XMMATRIX SetWorldViewProjection2D(UINT num);
	void SetGlobalBuffer(GlobalBuffer12* gb, ComPtr<ID3D12Resource> GlobalBuffer);
	void SetWorldMatrix(XMMATRIX WorldMatrix, ComPtr<ID3D12Resource> WorldBuffer);
	void SetViewMatrix(XMMATRIX ViewMatrix, ComPtr<ID3D12Resource> ViewBuffer);
	void SetProjectMatrix(XMMATRIX ProjectionMatrix, ComPtr<ID3D12Resource> ProjectionBuffer);
	void SetLight(LIGHT12* light, ComPtr<ID3D12Resource> LightBuffer);

	//ComPtr<ID3D12Resource> LoadTexture2D(ComPtr<ID3D12Resource> m_TextureUpLoad, ComPtr<ID3D12DescriptorHeap> m_DescHeap, const wchar_t* TextureName,UINT num,UINT format);
	ComPtr<ID3DBlob> CreateVertexShader(const wchar_t* ShaderName);
	ComPtr<ID3DBlob> CreatePixelShader(const wchar_t* ShaderName);

	ComPtr<ID3D12DescriptorHeap> CreateImGuiHeap();

	ComPtr<ID3D12Debug> GetDebug12() { return DebugController12; }
	ComPtr<ID3D12Device8> GetDevice() { return m_Device; }
	ComPtr<ID3D12GraphicsCommandList4> GetCommandList() { return m_CommandList; }
	ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return m_CommandAllocator; }
	ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_CommandQueue; }
	ComPtr<IDXGISwapChain3> GetSwapChain() { return m_SwapChain; }
	ComPtr<ID3D12Resource> GetRTarget(UINT backBufferIndex) { return m_RenderTargets[backBufferIndex]; }
	ComPtr<ID3D12DescriptorHeap> GetRTHeap() { return m_RtvHeap; }
	ComPtr<ID3D12Resource> GetShadowBuffer() { return m_ShadowBuffer; }

	ComPtr<ID3D12DescriptorHeap> GetImGuiHeap() { return m_ImGuiHeap; }
};