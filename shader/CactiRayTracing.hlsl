#include "RayTracingHeader.hlsli"

StructuredBuffer<VERTEX_RT_3D12> g_CactiVertex : register(t13);
StructuredBuffer<uint> g_CactiIndex : register(t14);
Texture2D g_CactiTexture : register(t15);

[shader("closesthit")]void CactiHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    VERTEX_RT_3D12 VertexRT = GetVertexRT(g_CactiVertex, g_CactiIndex, attr);
    
    float4 TextureColor = g_CactiTexture.SampleLevel(g_SamplerState, VertexRT.TexCoord, 0.0f);
    
    payload.color = TextureColor.xyz * VertexRT.Diffuse.xyz;
    
    if (!g_Light.flag)
    {
        float3 LightDir = normalize(g_Light.direction);
        float DotResult = dot(VertexRT.Normal, LightDir);
        DotResult = saturate(DotResult);
        payload.color = payload.color * DotResult;
    }
}