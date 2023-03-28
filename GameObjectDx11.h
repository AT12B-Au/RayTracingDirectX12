#pragma once
#include "AppDx11.h"
#include <stdexcept>

class GameObjectDx11
{
protected:
	bool m_Destory11 = false;

	ID3D11VertexShader* m_VertexShader11 = NULL;
	ID3D11PixelShader* m_PixelShader11 = NULL;
	ID3D11InputLayout* m_VertexLayout11 = NULL;

	D3DXVECTOR3 m_Position11;
	D3DXVECTOR3 m_Rotation11;
	D3DXVECTOR3 m_Scale11;

	float speed11;

	D3DXVECTOR3 m_Velocity11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);		//速度

public:
	GameObjectDx11() {}				//コンストラクタ
	virtual ~GameObjectDx11() {}		//デストラクター（仮想関数）

	virtual void Init(UINT version) = 0;	//純粋仮想関数
	virtual void UnInit(UINT version) = 0;
	virtual void Update(UINT version) = 0;
	virtual void Draw(UINT version) = 0;

	D3DXVECTOR3 GetPosition11() { return m_Position11; }
	//D3DXVECTOR3 GetRotation() { return m_Rotation; }
	D3DXVECTOR3 GetScale11() { return m_Scale11; }

	void SetPosition11(D3DXVECTOR3 Position)
	{
		m_Position11 = Position;
	}

	void SetRotation11(D3DXVECTOR3 Rotation)
	{
		m_Rotation11 = Rotation;
	}

	void SetScale11(D3DXVECTOR3 scale)
	{
		m_Scale11 = scale;
	}

	void SetDestroy11() { m_Destory11 = true; }

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

	bool Destroy11(UINT version)
	{
		if (m_Destory11)
		{
			UnInit(version);
			delete this;
			return true;
		}
		else
		{
			return false;
		}
	}
};