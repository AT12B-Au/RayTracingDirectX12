#include "RayTracingHeader.hlsli"

StructuredBuffer<VERTEX_RT_3D12> g_BoxVertex : register(t4);
StructuredBuffer<uint> g_BoxIndex : register(t5);
Texture2D g_BoxTexture : register(t6);

[shader("closesthit")]void BoxHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    VERTEX_RT_3D12 VertexRT = GetVertexRT(g_BoxVertex, g_BoxIndex, attr);
    
    payload.color = g_BoxTexture.SampleLevel(g_SamplerState, VertexRT.TexCoord, 0.0f);
    
    if (!g_Light.flag)
    {
        float3 LightDir = normalize(g_Light.direction);
        float DotResult = dot(VertexRT.Normal, LightDir);
        DotResult = saturate(DotResult);
        payload.color = payload.color * DotResult;
    }

    float3 rayDirW = WorldRayDirection();

    float3 rayOriginW = WorldRayOrigin();

    float hitT = RayTCurrent();

    float3 hitPos = rayOriginW + rayDirW * hitT;

    float3 refDir = reflect(rayDirW, VertexRT.Normal);
    
    RayDesc ray;
    ray.Origin = hitPos;
    ray.Direction = refDir;
    ray.TMin = 0.01f;
    ray.TMax = 100000;

    // step-4 レイを飛ばす
    if (payload.reflection == 0)
    {
        // このレイがリフレクションレイでないなら
        Payload reflectionPayload;
        reflectionPayload.color = 0.0f;
        
        uint RayMask = ~(0x08);

        // このレイはリフレクションレイなのでフラグを立てる
        reflectionPayload.reflection = 1;
        TraceRay(
            g_scene,
            0,
            RayMask,
            0, //ヒットグループのオフセット番号が0
                        //つまり、ポリゴンと衝突するとchs関数が呼ばれる！
            1,
            0,
            ray,
            reflectionPayload
        );
        
        // step-5 反射カラーの合成
        payload.color = payload.color * 0.7f + reflectionPayload.color * 0.3f;
    }
}