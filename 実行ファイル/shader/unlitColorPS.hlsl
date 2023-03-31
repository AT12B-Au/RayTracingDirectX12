//ピクセルシェーダー
#include "Header.hlsli" //必ずインクルード
//
//in は入力されてくるデーター
//out は出力するデータ
//
void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//1 ピクセル分の色を決定して出力する
	//入力されたピクセルの色をそのまま出力
	outDiffuse = In.Diffuse;

}