//ピクセルシェーダー
#include "Header.hlsli" //必ずインクルード
//
//in は入力されてくるデーター
//out は出力するデータ
//

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    outDiffuse = float4(0.0f,0.0f,0.0f,0.5f);
}