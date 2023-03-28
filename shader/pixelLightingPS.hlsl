#include "Header.hlsli"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//ピクセルの法線を正規化
	float4 normal = normalize(In.Normal);
    float light;
    if (-dot(normal.xyz, Light.Direction.xyz) >= 0.0f)
		//光源計算をする
        light = 0.5 - 0.5 * dot(normal.xyz, Light.Direction.xyz);
	else
		//光源計算をする
        light = 0.5 - 0.5 * dot(-normal.xyz, Light.Direction.xyz);

	//スペキュラの計算
	//カメラからピクセルへ向かうベクトル
	float3 eyev = In.WorldPosition.xyz - g_CameraPosition.xyz;
	eyev = normalize(eyev); //正規化する
	//光の反射ベクトルを計算
	float3 refv = reflect(Light.Direction.xyz, normal.xyz);
	refv = normalize(refv); //正規化する
	
	float specular = -dot(eyev, refv); //鏡面反射の計算
	specular = saturate(specular); //値をサチュレート
	specular = pow(specular, 30); //ここでは３０乗してみる

	//テクスチャのピクセル色を取得
	outDiffuse = 
		g_Texture.Sample(g_SamplerState, In.TexCoord);

    outDiffuse.rgb *=
		In.Diffuse.rgb * light; //明るさと色を乗算
	outDiffuse.a *= 
		1.0f; //α別計算

	//スペキュラ値をデフューズとして足しこむ
	outDiffuse.rgb += specular;
}


