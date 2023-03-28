#pragma once
#include "AppDx11.h"
#include "AppDx12.h"
#include <stdexcept>
#include <DirectXMath.h>

class GameObject
{
protected:
	
	//DirectX11

	bool m_Destory11 = false;

	ComPtr<ID3D11VertexShader> m_VertexShader11 = nullptr;
	ComPtr<ID3D11PixelShader> m_PixelShader11 = nullptr;
	ComPtr<ID3D11InputLayout> m_VertexLayout11 = nullptr;

	D3DXVECTOR3 m_Position11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 m_Scale11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	float speed11 = 0.0f;

	D3DXVECTOR3 m_Velocity11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);		//速度

	//DirectX12

	bool m_Destory12 = false;
	XMFLOAT3 m_Position12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_Scale12 = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float speed12;

	XMFLOAT3 m_Velocity12 = XMFLOAT3(0.0f, 0.0f, 0.0f);		//速度

	//シェーダー
	ComPtr<ID3DBlob> m_VertexShader12 = nullptr;
	ComPtr<ID3DBlob> m_PixelShader12 = nullptr;

	float alpha = 1.0f;

	//レイトレーシング
	int m_InitObjectNum = 1;

public:
	GameObject() {}				//コンストラクタ
	virtual ~GameObject() {}		//デストラクター（仮想関数）

	virtual void Init(UINT version) = 0;	//純粋仮想関数
	virtual void UnInit(UINT version) = 0;
	virtual void Update(UINT version) = 0;
	virtual void Draw(UINT version) = 0;

	D3DXVECTOR3 GetPosition11() { return m_Position11; }
	D3DXVECTOR3 GetRotation11() { return m_Rotation11; }
	D3DXVECTOR3 GetScale11() { return m_Scale11; }
	XMFLOAT3 GetPosition12() { return m_Position12; }
	XMFLOAT3 GetRotation12() { return m_Rotation12; }
	XMFLOAT3 GetScale12() { return m_Scale12; }

	void SetPosition(UINT version,float PositionX,float PositionY,float PositionZ)
	{
		if (version == 0)
			m_Position11 = D3DXVECTOR3(PositionX, PositionY, PositionZ);
		else if (version == 1)
			m_Position12 = XMFLOAT3(PositionX, PositionY, PositionZ);
	}

	void SetRotation(UINT version, float RotationX, float RotationY, float RotationZ)
	{
		if (version == 0)
			m_Rotation11 = D3DXVECTOR3(RotationX, RotationY, RotationZ);
		else if (version == 1)
			m_Rotation12 = XMFLOAT3(RotationX, RotationY, RotationZ);
	}

	void SetScale(UINT version, float ScaleX, float ScaleY, float ScaleZ)
	{
		if (version == 0)
			m_Scale11 = D3DXVECTOR3(ScaleX, ScaleY, ScaleZ);
		else if (version == 1)
			m_Scale12 = XMFLOAT3(ScaleX, ScaleY, ScaleZ);
	}

	void SetDestroy(UINT version)
	{
		if (version == 0)
			m_Destory11 = true;
		else if (version == 1)
			m_Destory12 = true;
	}

	bool GetDestroy(UINT version)
	{
		if (version == 0)
			return m_Destory11;
		else if (version == 1)
			return m_Destory12;
	}

	bool Destroy(UINT version)
	{
		if (m_Destory11 || m_Destory12)
		{
			UnInit(version);
			return true;
		}
		else
		{
			return false;
		}
	}

	D3DXVECTOR3 GetForward11(int num)	//前方向ベクトル取得
	{
		D3DXMATRIX rot;
		D3DXMatrixRotationYawPitchRoll(&rot, m_Rotation11.y, m_Rotation11.x, m_Rotation11.z);

		D3DXVECTOR3 forward_x;
		D3DXVECTOR3 forward_y;
		D3DXVECTOR3 forward_z;

		forward_x.x = rot._11;
		forward_x.y = rot._12;
		forward_x.z = rot._13;

		forward_y.x = rot._21;
		forward_y.y = rot._22;
		forward_y.z = rot._23;

		forward_z.x = rot._31;
		forward_z.y = rot._32;
		forward_z.z = rot._33;

		if (num == 0)
		{
			return forward_x;
		}
		else if (num == 1)
		{
			return forward_y;
		}
		else if (num == 2)
		{
			return forward_z;
		}
		else
		{
			throw std::runtime_error("WrongNum");
		}
	}

	XMFLOAT3 GetForward12(int num)	//前方向ベクトル取得
	{
		XMMATRIX rot;
		rot = XMMatrixRotationRollPitchYaw(m_Rotation12.x, m_Rotation12.y, m_Rotation12.z);

		XMFLOAT3 forward_x;
		XMFLOAT3 forward_y;
		XMFLOAT3 forward_z;

		XMFLOAT4X4 rot_4x4;

		XMStoreFloat4x4(&rot_4x4, rot);

		forward_x.x = rot_4x4._11;
		forward_x.y = rot_4x4._12;
		forward_x.z = rot_4x4._13;

		forward_y.x = rot_4x4._21;
		forward_y.y = rot_4x4._22;
		forward_y.z = rot_4x4._23;

		forward_z.x = rot_4x4._31;
		forward_z.y = rot_4x4._32;
		forward_z.z = rot_4x4._33;

		if (num == 0)
		{
			return forward_x;
		}
		else if (num == 1)
		{
			return forward_y;
		}
		else if (num == 2)
		{
			return forward_z;
		}
		else
		{
			throw std::runtime_error("WrongNum");
		}
	}

	void SetAlpha(float value)
	{
		alpha = value;
	}

	void ConvertTransform(D3D12_RAYTRACING_INSTANCE_DESC& desc, XMFLOAT3X4 trans)
	{
		desc.Transform[0][0] = trans._11;
		desc.Transform[0][1] = trans._12;
		desc.Transform[0][2] = trans._13;
		desc.Transform[0][3] = trans._14;

		desc.Transform[1][0] = trans._21;
		desc.Transform[1][1] = trans._22;
		desc.Transform[1][2] = trans._23;
		desc.Transform[1][3] = trans._24;

		desc.Transform[2][0] = trans._31;
		desc.Transform[2][1] = trans._32;
		desc.Transform[2][2] = trans._33;
		desc.Transform[2][3] = trans._34;
	}

	int GetInitObjectNum() { return m_InitObjectNum; }
};