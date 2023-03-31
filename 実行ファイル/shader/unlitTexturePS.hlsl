//ピクセルシェーダー
#include "Header.hlsli" //必ずインクルード
//
//in は入力されてくるデーター
//out は出力するデータ
//

//DirectXのテクスチャの設定を受け継ぐ
Texture2D g_Texture : register(t0);//テクスチャー０番
//DirectXのサンプラーステート設定を受け継ぐ
SamplerState g_SamplerState : register(s0);//テクスチャサンプ

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//このピクセルに使われるテクスチャの色を取得
	outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
	//入力されたピクセルの色をそのまま出力
	outDiffuse *= In.Diffuse;

	////グレースケールにしてみる
	//outDiffuse.rgb = 
	//	  outDiffuse.r * 0.3
	//	+ outDiffuse.g * 0.6
	//	+ outDiffuse.b * 0.1;

	//////セピア調変換してみる
	//float4 sepia = float4(0.960, 0.784, 0.580, 1);
	//outDiffuse *= sepia;

	////outDiffuse = pow(outDiffuse,6);
	
	
	//2022_11_07追加	//距離を色として
    //float2 len = float2(0.5f, 0.5f);		//テクスチャの中心座標
    //len -= In.TexCoord.xy;					//中心座標からテクスチャ座標を引く
    //len = length(len);						//ベクトルの長さを計算
	
    //outDiffuse.rgb = len.x;					//距離をピクセルのRGBへ出力

	//2022_11_07追加	//色の反転版　真ん中が明るい
    //float2 len = float2(0.5f, 0.5f); //テクスチャの中心座標
    //len -= In.TexCoord.xy; //中心座標からテクスチャ座標を引く
    //len = 1.0f - length(len); //ベクトルの長さを計算
	
    //outDiffuse.rgb = pow(len.x, 4); //距離をピクセルのRGBへ出力
	
	//2022_11_07追加	//円形ウィンドマスク
    //float2 len = float2(0.5f, 0.5f); //テクスチャの中心座標
    //len -= In.TexCoord.xy; //中心座標からテクスチャ座標を引く
    //len = length(len); //ベクトルの長さを計算
    //if (len.x < 0.3f)
    //{
    //    outDiffuse.rgb *= 1.0f;
    //}
    //else
    //{
    //    outDiffuse.rgb = 0.0f;
    //}
	
	////2022_11_07追加	//RGBシフト
 //   float4 colbuf = 0;
 //   float offset = (1.0f / 650.0f) * Parameter.x; //650 テクスチャのサイズ	//Parameter.xが表すピクセル分のテクスチャ座標

	////R要素を取得(offset分ずれた場所から)
 //   colbuf.r = g_Texture.Sample(g_SamplerState, float2(In.TexCoord.x - offset, In.TexCoord.y)).r;
	////G要素を取得
 //   colbuf.g = g_Texture.Sample(g_SamplerState, In.TexCoord).g;
	////B要素を取得
 //   colbuf.b = g_Texture.Sample(g_SamplerState, float2(In.TexCoord.x + offset, In.TexCoord.y)).b;
	
 //   outDiffuse.rgb = colbuf.rgb;
	
	outDiffuse.a = 1.0f;
}