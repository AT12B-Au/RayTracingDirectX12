#pragma once
#include "AppDx12.h"

class GameObjectDx12
{
protected:
	bool m_Destory12 = false;
	XMFLOAT3 m_Position12;
	XMFLOAT3 m_Rotation12;
	XMFLOAT3 m_Scale12;

	float speed12;

	XMFLOAT3 m_Velocity12 = XMFLOAT3(0.0f, 0.0f, 0.0f);		//速度

	//シェーダー
	ComPtr<ID3DBlob> m_VertexShader12;
	ComPtr<ID3DBlob> m_PixelShader12;

public:
	GameObjectDx12() {}				//コンストラクタ
	virtual ~GameObjectDx12() {}		//デストラクター（仮想関数）

	virtual void Init(UINT version) = 0;	//純粋仮想関数
	virtual void UnInit(UINT version) = 0;
	virtual void Update(UINT version) = 0;
	virtual void Draw(UINT version) = 0;

	XMFLOAT3 GetPosition12() { return m_Position12; }
	XMFLOAT3 GetRotation12() { return m_Rotation12; }
	XMFLOAT3 GetScale12() { return m_Scale12; }

	void SetPosition12(XMFLOAT3 Position)
	{
		m_Position12 = Position;
	}

	void SetRotation12(XMFLOAT3 Rotation)
	{
		m_Rotation12 = Rotation;
	}

	void SetScale12(XMFLOAT3 scale)
	{
		m_Scale12 = scale;
	}

	void SetDestroy12() { m_Destory12 = true; }

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

	bool Destroy12(UINT version)
	{
		if (m_Destory12)
		{
			UnInit(version);
			//delete this;
			return true;
		}
		else
		{
			return false;
		}
	}
};