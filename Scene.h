#pragma once

#include <list>
#include <vector>
#include <typeinfo>
#include "GameObject.h"
#include "RendererManager.h"
#include "ImGui11.h"
#include "ImGui12.h"

#include <boost/container/vector.hpp>

//#define DRAW_CAMERA	(0)
//#define DRAW_3D		(1)
//#define DRAW_2D		(2)
//#define DRAW_MUSIC	(3)

enum 
{
	DRAW_CAMERA,
	DRAW_3D,
	DRAW_RAYTRACING,
	DRAW_2D,
	DRAW_MUSIC,
	TOTAL_DRAW,
};

class Scene
{
protected:
	std::list<GameObject*>	m_GameObject[TOTAL_DRAW];	//STLのリスト構造
	int m_RTObjectNum = 0;
	boost::container::vector<D3D12_RAYTRACING_INSTANCE_DESC> InstanceContainer;

public:
	Scene() {}
	virtual ~Scene() {}

	template<typename T>	//テンプレート関数
	T* AddGameObject(int i,UINT version)
	{
		T* gameObject = new T();

		gameObject->Init(version);
		m_GameObject[i].push_back(gameObject);

		if (i == DRAW_RAYTRACING)
			m_RTObjectNum++;

		return gameObject;
		
	}

	int GetRTObjectNum() { return m_RTObjectNum; }

	std::list<GameObject*> GetRTObjectList() { return m_GameObject[DRAW_RAYTRACING]; }

	template<typename T>
	T* GetGameObject(int Layer,UINT version)
	{
		#pragma omp parallel for
		for (GameObject* object : m_GameObject[Layer])
		{
			if (typeid(*object) == typeid(T))		//型を調べる(RTTI動的型情報)
			{
				return (T*)object;
			}
		}
		return NULL;

	}


	template <typename T>
	boost::container::vector<T*> GetGameObjects(int Layer,UINT version)
	{
		boost::container::vector<T*> objects;	//STLの配列

		#pragma omp parallel for
		for (GameObject* object : m_GameObject[Layer])
		{
			if (typeid(*object) == typeid(T))
			{
				objects.push_back((T*)object);
			}
		}
		return objects;

	}

	virtual void Init(UINT version)
	{

	};

	virtual void UnInit(UINT version) 
	{
		#pragma omp parallel for
		for (int i = 0; i < TOTAL_DRAW; i++)
		{
			#pragma omp parallel for
			for (GameObject* object : m_GameObject[i])
			{
				object->UnInit(version);
				//delete object;
			}
			m_GameObject[0].clear();				//リストのクリア
		}

	};

	virtual void Update(UINT version) 
	{
		#pragma omp parallel for
		for (int i = 0; i < TOTAL_DRAW; i++)
		{
			#pragma omp parallel for
			for (GameObject* object : m_GameObject[i])
			{
				object->Update(version);			   //ポリモフィズム（多態性）
			}

			m_GameObject[i].remove_if([&](GameObject* object) {return object->Destroy(version); });
			//ラムダ式
		}
	}

	virtual void Draw(UINT version) 
	{
		if (version == 0)
			ImGui11::SetImGuiOnOff11();
		else if (version == 1)
			ImGui12::SetImGuiOnOff12();

		#pragma omp parallel for
		for (int i = 0; i < TOTAL_DRAW; i++)
		{
			#pragma omp parallel for
			for (GameObject* object : m_GameObject[i])
			{
				object->Draw(version);
			}
		}
	}

	virtual void DrawShadow(UINT version)
	{
		#pragma omp parallel for
		for (GameObject* object : m_GameObject[DRAW_3D])
		{
			object->Draw(version % 2);
		}
		
	}

	virtual void SetAlpha(UINT version,float value)
	{
		#pragma omp parallel for
		for (int i = 1; i < TOTAL_DRAW; i++)
		{
			#pragma omp parallel for
			for (GameObject* object : m_GameObject[i])
			{
				object->SetAlpha(value);
			}
		}
	}

	boost::container::vector<D3D12_RAYTRACING_INSTANCE_DESC>& GetInstanceContainer() { return InstanceContainer; }
};