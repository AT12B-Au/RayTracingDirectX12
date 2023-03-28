#pragma once
#include "RendererManager.h"
#include <unordered_map>

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"

#include <boost/container/vector.hpp>

//変形後頂点構造体
struct DEFORM_VERTEX
{
	aiVector3D Position;
	aiVector3D Normal;
	int BoneNum;
	std::string BoneName[4];	//本来はボーンインデックスで管理するべき
	float BoneWeight[4];
};

//ボーン構造体
struct BONE
{
	aiMatrix4x4 Matrix;
	aiMatrix4x4 AnimationMatrix;
	aiMatrix4x4 TransMatrix;
	aiMatrix4x4 OffsetMatrix;
};

class AnimationModel11
{
private:
	const aiScene* m_AiScene = NULL;
	std::unordered_map<std::string, const aiScene*> m_Animation;

	//std::vector<DEFORM_VERTEX>* m_DeformVertex;
	boost::container::vector<DEFORM_VERTEX>* m_DeformVertex;		//変形後頂点データ
	std::unordered_map<std::string, BONE> m_Bone;	//ボーンデータ(名前で参照)

	//DirectX11
	ComPtr<ID3D11Buffer>* m_VertexBuffer11;
	ComPtr<ID3D11Buffer>* m_IndexBuffer11;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Texture11;

	void CreateBone(aiNode* node);
	void UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix);

public:
	void Load(const char* FileName);
	void LoadAnimation(const char* FileName, const char* AnimationName);
	void UnLoad();
	void Update(const char* AnimationName1, const char* AnimationName2, float BlendRate, int frame);
	void Draw();
};