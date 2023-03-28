#include "RayTracingHeader.hlsli"

StructuredBuffer<VERTEX_RT_3D12> g_FloorVertex : register(t7);
StructuredBuffer<uint> g_FloorIndex : register(t8);
Texture2D g_FloorTexture : register(t9);

// �V���h�E���C���΂�
bool ShadowRay(float3 origin, float3 direction)
{
    // �V���h�E���C�����
    RayDesc ray;

    // �V���h�E���C���΂��ꏊ
    ray.Origin = origin;

    ray.Direction = direction;

    // ���C�̍ŏ������ƍő勗��
    ray.TMin = 0.01f;
    ray.TMax = 100000.0f;
    
    RAY_FLAG flags = RAY_FLAG_NONE;
    flags |= RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

    uint RayMask = ~(0x08);
    
    // �V���h�E���C���΂�
    Payload ShadowRay;
    ShadowRay.hit = true;
    
    TraceRay(
        g_scene, // ���C�g�����[���h
        flags, //
        RayMask,
        0, // �q�b�g�O���[�v�̃I�t�Z�b�g�ԍ�
        1,
        1, // �~�X�V�F�[�_�[�̔ԍ�
        ray, // ���C
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

    // step-4 ���C���΂�
    if (payload.reflection == 0)
    {
        // ���̃��C�����t���N�V�������C�łȂ��Ȃ�
        Payload reflectionPayload;
        reflectionPayload.color = 0.0f;
        
        uint RayMask = ~(0x08);

        // ���̃��C�̓��t���N�V�������C�Ȃ̂Ńt���O�𗧂Ă�
        reflectionPayload.reflection = 1;
        TraceRay(
            g_scene,
            0,
            RayMask,
            0, //�q�b�g�O���[�v�̃I�t�Z�b�g�ԍ���0
                        //�܂�A�|���S���ƏՓ˂����chs�֐����Ă΂��I
            1,
            0,
            ray,
            reflectionPayload
        );
        
        // step-5 ���˃J���[�̍���
        payload.color = payload.color * 0.7f + reflectionPayload.color * 0.3f;
        
        
        //�V���h�E����
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