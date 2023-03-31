//頂点シェーダーへ入力されるデータを構造体の形で表現
//これは頂点バッファの内容そのもの
struct VS_IN
{
    float4 Position : POSITION0; //セマンティクスは頂点レイアウトに対応
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};
//ピクセルシェーダーへ入力されるデータを構造体の形で表現
struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

//ライトオブジェクト構造体とコンスタントバッファ
struct LIGHT
{
    bool Enable;
    bool3 Dummy; //配置アドレスを4の倍数にすると効率がいいので調整用
    float4 Direction; //実はC言語でも同じでVisualStudioがやってくれている。
    float4 Diffuse;
    float4 Ambient;
};
cbuffer GlobalBuffer11 : register(b0)
{
    //matrix g_Wvp; //ワールドから射影までの変換行列
    //float4 g_LightDir; //ライトの方向ベクトル	
    float4 g_Eye; //カメラ（視点）
}

//このファイルは他のシェーダーファイルへインクルードされます
// 各種マトリクスを受け取る変数を用意
cbuffer WorldBuffer : register(b1) //定数バッファ 0 番
{
    matrix World;
}
cbuffer ViewBuffer : register(b2) //定数バッファ 1 番
{
    matrix View;
}
cbuffer ProjectionBuffer : register(b3) //定数バッファ 2 番
{
    matrix Projection;
}
cbuffer LightBuffer : register(b5) //コンスタントバッファ４番とする
{
    LIGHT Light; //ライト構造体
}
//cbuffer CameraBuffer : register(b6) //バッファの５番とする
//{
//    float4 CameraPosition; //カメラ座標を受けとる変数
//}
//cbuffer ParameterBuffer : register(b7)
//{
//    float4 Parameter;
//}
