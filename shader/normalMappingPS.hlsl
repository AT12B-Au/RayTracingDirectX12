//ピクセルシェーダー
#include "Header.hlsli" //必ずインクルード
//
//in は入力されてくるデーター
//out は出力するデータ
//

//DirectXのテクスチャの設定を受け継ぐ
Texture2D g_Texture : register(t0);//テクスチャー０番
Texture2D g_TextureNormal : register(t1); //テクスチャー１番
//DirectXのサンプラーステート設定を受け継ぐ
SamplerState g_SamplerState : register(s1);//テクスチャサンプ

void main(in PS_IN In, out float4 outDiffuse : SV_Target0)
{
    float4 normalMap = g_TextureNormal.Sample(g_SamplerState, In.TexCoord);
    
    normalMap = normalMap * 2.0f - 1.0f;
    
    float4 normal;
    normal.x = -normalMap.x;
    normal.y = normalMap.z;
    normal.z = normalMap.y;
    normal.w = 0.0f;
    
   //ピクセルの法線を正規化
    normal = normalize(normal);
	//光源計算をする
    float light = -dot(normal.xyz, Light.Direction.xyz);

	//スペキュラの計算
	//カメラからピクセルへ向かうベクトル
    float3 eyev = In.WorldPosition.xyz - g_CameraPosition.xyz;
    eyev = normalize(eyev); //正規化する

	////光の反射ベクトルを計算
    //float3 refv = reflect(Light.Direction.xyz, normal.xyz);
    //refv = normalize(refv); //正規化する

	//ハーフベクトル作成
    //float3 halfv = eyev + Light.Direction.xyz;
    //halfv = normalize(halfv); //正規化する

	//鏡面反射の計算
    float specular = -dot(eyev, normal.xyz);
    specular = saturate(specular); //値をサチュレート
    specular = pow(specular, 30); //ここでは３０乗してみる

	//テクスチャのピクセル色を取得
    outDiffuse =
		g_Texture.Sample(g_SamplerState, In.TexCoord);

    outDiffuse.rgb *=
		In.Diffuse.rgb * light; //明るさと色を乗算
    outDiffuse.a *=
		In.Diffuse.a; //α別計算

	//スペキュラ値をデフューズとして足しこむ
    outDiffuse.rgb += specular;
}