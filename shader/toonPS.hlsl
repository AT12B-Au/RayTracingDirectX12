//thsozai ==>前回までの状態　ZIP　　パスワード　nabe

#include "Header.hlsli"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//ピクセルの法線を正規化
    float4 normal = normalize(In.Normal);
	//光源計算をする
    float light = -dot(normal.xyz, Light.Direction.xyz);
    
    if (light < 0.6f)
        light = 0.3f;
    else if (light >= 0.6f && light <= 0.8f)
        light = 0.7f;
    else if (light > 0.8f)
        light = 0.9f;

	//テクスチャのピクセル色を取得
    outDiffuse =
		g_Texture.Sample(g_SamplerState, In.TexCoord);

    outDiffuse.rgb *=
		In.Diffuse.rgb * light; //明るさと色を乗算
    outDiffuse.a *=
		In.Diffuse.a; //α別計算

	//スペキュラ値をデフューズとして足しこむ
    //outDiffuse.rgb += specular;
    
    //エッジの作成
    //ピクセルシェーダーの最後で処理をする
    //float3 eyev = In.WorldPosition.xyz - g_CameraPosition.xyz;     //視線ベクトル
    //eyev = normalize(eyev);     //正規化
    
    //float d = dot(eyev, In.Normal.xyz);
    
    //if (d >= -0.2f)
    //{
    //    //outDiffuse.rb = 0.0f;     //適当な値で明るさを落す
    //    outDiffuse.g = 20.0f;
    //}

}


