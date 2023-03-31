//�\����
struct VS_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    
    float4 ShadowPosition : POSITION1;  //���C�g�J�������猩���s�N�Z���̍��W
};

//���C�g�I�u�W�F�N�g�\���̂ƃR���X�^���g�o�b�t�@
struct LIGHT
{
    bool Enable;
    bool3 Dummy; //�z�u�A�h���X��4�̔{���ɂ���ƌ����������̂Œ����p
    float4 Direction; //����C����ł�������VisualStudio������Ă���Ă���B
    float4 Diffuse;
    float4 Ambient;
    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
};

struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float Dummy[3];
};

cbuffer GlobalBuffer11 : register(b0)
{
    //matrix g_Wvp; //���[���h����ˉe�܂ł̕ϊ��s��
    //float4 g_LightDir; //���C�g�̕����x�N�g��	
    float4 g_CameraPosition; //�J�����i���_�j
    float4 g_Parameter;
    MATERIAL material;
}

cbuffer WorldBuffer : register(b1)
{
    matrix World;
}

cbuffer ViewBuffer : register(b2)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b3)
{
    matrix Projection;
}

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}