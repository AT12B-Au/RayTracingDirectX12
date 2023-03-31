//構造体
struct VS_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    
    float4 ShadowPosition : POSITION1;  //ライトカメラから見たピクセルの座標
};

//ライトオブジェクト構造体とコンスタントバッファ
struct LIGHT
{
    bool Enable;
    bool3 Dummy; //配置アドレスを4の倍数にすると効率がいいので調整用
    float4 Direction; //実はC言語でも同じでVisualStudioがやってくれている。
    float4 Diffuse;
    float4 Ambient;
    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
};

struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float Dummy[3];
};

cbuffer GlobalBuffer11 : register(b0)
{
    //matrix g_Wvp; //ワールドから射影までの変換行列
    //float4 g_LightDir; //ライトの方向ベクトル	
    float4 g_CameraPosition; //カメラ（視点）
    float4 g_Parameter;
    MATERIAL material;
}

cbuffer WorldBuffer : register(b1)
{
    matrix World;
}

cbuffer ViewBuffer : register(b2)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b3)
{
    matrix Projection;
}

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}