#include "Header.hlsli"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//ピクセルの法線を正規化
	float4 normal = normalize(In.Normal);
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
	float3	halfv = eyev + Light.Direction.xyz;
	halfv = normalize(halfv); //正規化する


	//鏡面反射の計算
	float specular = -dot(halfv, normal.xyz);
	specular = saturate(specular); //値をサチュレート
	specular = pow(specular, 30); //ここでは３０乗してみる

	//リムライティング
	float rim = 1.0 + dot(eyev, normal.xyz); //視線と法線の内積を明るさに変換する
	rim = pow(rim, 3) * 2.0f; //スペキュラと同じような処理を適当に行う。
	rim = saturate(rim); //rimをサチュレートする


	//テクスチャのピクセル色を取得
	outDiffuse = 
		g_Texture.Sample(g_SamplerState, In.TexCoord);

	outDiffuse.rgb *=
		In.Diffuse.rgb * light; //明るさと色を乗算
	outDiffuse.a *= 
		In.Diffuse.a; //α別計算

	//スペキュラ値をデフューズとして足しこむ
	outDiffuse.rgb += specular;
	//リム値をデフューズとして足しこむ
	outDiffuse.g += rim;

}


