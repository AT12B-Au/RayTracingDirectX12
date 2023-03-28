#include "Header.hlsli"

Texture2D g_Tex : register(t0);
SamplerState g_Sampler : register(s0);

//ピクセルシェーダー
//
void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    outDiffuse = g_Tex.Sample(g_Sampler, In.TexCoord);
    outDiffuse *= In.Diffuse;
}