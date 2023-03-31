//ピクセルシェーダー
#include "Header.hlsli" //必ずインクルード
//
//in は入力されてくるデーター
//out は出力するデータ
//

//DirectXのテクスチャの設定を受け継ぐ
Texture2D g_Texture : register(t0); //テクスチャー
Texture2D g_TextureNormal : register(t1); //ノーマルマップ
Texture2D g_TextureShadowDepth : register(t2); //シャドウ
//DirectXのサンプラーステート設定を受け継ぐ
SamplerState g_SamplerState : register(s1); //テクスチャサンプ

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
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
    
    In.ShadowPosition.xyz /= In.ShadowPosition.w; //投影面でのピクセルの座標つくる
    In.ShadowPosition.x = In.ShadowPosition.x * 0.5f + 0.5f; //テクスチャ座標に変換 x * ? + ?
    In.ShadowPosition.y = -In.ShadowPosition.y * 0.5f + 0.5f; //Yは符号を反転しておく -y * ? + ?
    
    //シャドウマップテクスチャより、ライトカメラからピクセルまでの距離(深度値)を取得
    float depth = g_TextureShadowDepth.Sample(g_SamplerState, In.ShadowPosition.xy);
    
    //取得値が計算上のピクセルへの距離より小さい
    if (depth < In.ShadowPosition.z - 0.01f)     //0.01はZファイティング補正値
    {
        outDiffuse.rgb *= 0.5f; //色を暗くする
    }
}