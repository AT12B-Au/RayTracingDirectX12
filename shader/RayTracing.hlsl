#include "RayTracingHeader.hlsli"

StructuredBuffer<VERTEX_RT_3D12> g_BallVertex : register(t1);
StructuredBuffer<uint> g_BallIndex : register(t2);
Texture2D g_BallTexture : register(t3);

[shader("closesthit")] void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    VERTEX_RT_3D12 VertexRT = GetVertexRT(g_BallVertex, g_BallIndex, attr);
    
    float4 TextureColor = g_BallTexture.SampleLevel(g_SamplerState, VertexRT.TexCoord, 0.0f);
    
    payload.color = TextureColor.xyz * VertexRT.Diffuse.xyz;
}