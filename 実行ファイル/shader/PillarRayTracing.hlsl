#include "RayTracingHeader.hlsli"

StructuredBuffer<VERTEX_RT_3D12> g_PillarVertex : register(t10);
StructuredBuffer<uint> g_PillarIndex : register(t11);
Texture2D g_PillarTexture : register(t12);

[shader("closesthit")]void PillarHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    VERTEX_RT_3D12 VertexRT = GetVertexRT(g_PillarVertex, g_PillarIndex, attr);
    
    float4 TextureColor = g_PillarTexture.SampleLevel(g_SamplerState, VertexRT.TexCoord, 0.0f);
    
    payload.color = TextureColor.xyz * VertexRT.Diffuse.xyz;
    
    if (!g_Light.flag)
    {
        float3 LightDir = normalize(g_Light.direction);
        float DotResult = dot(VertexRT.Normal, LightDir);
        DotResult = saturate(DotResult);
        //DotResult = pow(DotResult, 0.3f);
        payload.color = payload.color * DotResult;
    }
}