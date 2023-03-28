struct Payload
{
    float3 color;
    bool hit; // 衝突フラグ
    int reflection; // リフレクションフラグ
};

struct CameraRT
{
    float4x4 CameraRot;
    float3 pos;
    float aspect;
    float f_near;
    float f_far;
};

struct VERTEX_RT_3D12
{
    float3 Position;
    float3 Normal;
    float4 Diffuse;
    float2 TexCoord;
};

cbuffer CameraBuffer : register(b0)
{
    CameraRT g_Camera; // カメラ
};

struct LightRT
{
    float3 direction; //平行光源のみ
    float4 color;
    float4 AmbientColor;
    float3 PointLightPos;
    uint ShadowRayCount;
    bool flag; //平行光源か点光源か
};

cbuffer LightBuffer : register(b1)
{
    LightRT g_Light; // カメラ
};

RaytracingAccelerationStructure g_scene : register(t0);  
RWTexture2D<float4> g_output : register(u0);

SamplerState g_SamplerState : register(s0);

[shader("raygeneration")]void RayGen()
{
    uint3 rayIndex = DispatchRaysIndex();
    uint3 rayDims = DispatchRaysDimensions();

    float2 pos = float2(rayIndex.xy);
    float2 dims = float2(rayDims.xy);

    float2 d = ((pos / dims) * 2.0f - 1.0f);
    float aspect = dims.x / dims.y;

    RayDesc desc;
    desc.Origin = g_Camera.pos;
    
    desc.Direction = normalize(float3(d.x * aspect, -d.y, 1));
    desc.Direction = mul(g_Camera.CameraRot, desc.Direction);

    desc.TMin = g_Camera.f_near;
    desc.TMax = g_Camera.f_far;

    Payload payload;
    
    payload.reflection = 0;
    
    uint RayMask = ~(0x08);
    
    TraceRay(g_scene, 0, RayMask, 0, 0, 0, desc, payload);
    g_output[rayIndex.xy] = float4(payload.color, 1);
}


[shader("miss")]void Miss(inout Payload payload)
{
    //float4 TextureColor = g_BackgroundTexture.SampleLevel(g_SamplerState, WorldRayDirection().xy, 0.0f);
    
    payload.color = float3(0.1f,0.1f,0.1f);
}

[shader("miss")]void ShadowMiss(inout Payload payload)
{
    payload.hit = false;
}

VERTEX_RT_3D12 GetVertexRT(StructuredBuffer<VERTEX_RT_3D12> g_Vertex, StructuredBuffer<uint> g_Index, 
BuiltInTriangleIntersectionAttributes attr)
{
    VERTEX_RT_3D12 VertexRT;

    float3 barycentrics = float3(1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    
    // プリミティブIDを取得
    uint primID = PrimitiveIndex();

    // プリミティブIDから頂点番号を取得する
    uint v0_id = g_Index[primID * 3];
    uint v1_id = g_Index[primID * 3 + 1];
    uint v2_id = g_Index[primID * 3 + 2];
    float3 Position0 = g_Vertex[v0_id].Position;
    float3 Position1 = g_Vertex[v1_id].Position;
    float3 Position2 = g_Vertex[v2_id].Position;
    float3 position = barycentrics.x * Position0 + barycentrics.y * Position1 + barycentrics.z * Position2;
    
    position = mul(float4(position, 1.0f), ObjectToWorld4x3());

    float4 Diffuse0 = g_Vertex[v0_id].Diffuse;
    float4 Diffuse1 = g_Vertex[v1_id].Diffuse;
    float4 Diffuse2 = g_Vertex[v2_id].Diffuse;
    
    float2 TexCoord0 = g_Vertex[v0_id].TexCoord;
    float2 TexCoord1 = g_Vertex[v1_id].TexCoord;
    float2 TexCoord2 = g_Vertex[v2_id].TexCoord;
    
    float3 normal0 = g_Vertex[v0_id].Normal;
    float3 normal1 = g_Vertex[v1_id].Normal;
    float3 normal2 = g_Vertex[v2_id].Normal;
    float3 normal = barycentrics.x * normal0 + barycentrics.y * normal1 + barycentrics.z * normal2;
    
    normal = mul(normal, (float3x3) ObjectToWorld4x3());
    normal = normalize(normal);
    
    VertexRT.Position = position;
    VertexRT.Diffuse = barycentrics.x * Diffuse0 + barycentrics.y * Diffuse1 + barycentrics.z * Diffuse2;
    VertexRT.TexCoord = barycentrics.x * TexCoord0 + barycentrics.y * TexCoord1 + barycentrics.z * TexCoord2;
    VertexRT.Normal = normal;
    
    return VertexRT;
}