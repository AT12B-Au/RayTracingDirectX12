#include "RayTracingHeader.hlsli"

StructuredBuffer<VERTEX_RT_3D12> g_FloorVertex : register(t7);
StructuredBuffer<uint> g_FloorIndex : register(t8);
Texture2D g_FloorTexture : register(t9);

// シャドウレイを飛ばす
bool ShadowRay(float3 origin, float3 direction)
{
    // シャドウレイを作る
    RayDesc ray;

    // シャドウレイを飛ばす場所
    ray.Origin = origin;

    ray.Direction = direction;

    // レイの最小距離と最大距離
    ray.TMin = 0.01f;
    ray.TMax = 100000.0f;
    
    RAY_FLAG flags = RAY_FLAG_NONE;
    flags |= RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

    uint RayMask = ~(0x08);
    
    // シャドウレイを飛ばす
    Payload ShadowRay;
    ShadowRay.hit = true;
    
    TraceRay(
        g_scene, // レイトレワールド
        flags, //
        RayMask,
        0, // ヒットグループのオフセット番号
        1,
        1, // ミスシェーダーの番号
        ray, // レイ
        ShadowRay
    );
    
    return ShadowRay.hit;
}

[shader("closesthit")]void FloorHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    VERTEX_RT_3D12 VertexRT = GetVertexRT(g_FloorVertex, g_FloorIndex, attr);
    
    payload.color = g_FloorTexture.SampleLevel(g_SamplerState, VertexRT.TexCoord, 0.0f);

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
        
        
        //シャドウ部分
        //float3 lightPos = mul(float4(g_Light.PointLightPos, 1.0f), ObjectToWorld4x3());
        float3 test = g_Light.PointLightPos - VertexRT.Position;
        float3 direction = normalize(test);
        bool isShadow = ShadowRay(VertexRT.Position, g_Light.direction);
    
        if (isShadow)
        {
            payload.color *= 0.5f;
        }
    }
}