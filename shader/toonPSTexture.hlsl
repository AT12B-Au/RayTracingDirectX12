//thsozai ==>�O��܂ł̏�ԁ@ZIP�@�@�p�X���[�h�@nabe


//BlinnPhongLightingPS.hlsl
//BlinnPhongLightingVS.hlsl


#include "Header.hlsli"

Texture2D g_Texture : register(t0);
Texture2D g_TextureToon : register(t1);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//�s�N�Z���̖@���𐳋K��
    float4 normal = normalize(In.Normal);
	//�����v�Z������
    float light = -dot(normal.xyz, Light.Direction.xyz);
    
    //���邳���N�����v����@0.0 < light < 1.0
    light = clamp(light, 0, 1);     //�e�N�X�`���T�C�Y
    
    //UV���W���쐬   x = light y = 0.5f�Ƃ���
    float2 toonTexCoord = float2(light + g_Parameter.x, 0.1f);
    
    if (light < 0.6f)
    {
        light = 0.3f;
    }
    else if (light >= 0.6f && light <= 0.8f)
    {
        light = 0.7f;
    }
    else if (light > 0.8f)
    {
        light = 0.9f;
    }

	//�e�N�X�`���̃s�N�Z���F���擾
    outDiffuse =
		g_Texture.Sample(g_SamplerState, In.TexCoord);
    float4 toon = g_TextureToon.Sample(g_SamplerState, toonTexCoord);

    outDiffuse.rgb *=
		In.Diffuse.rgb  * toon.rgb; //���邳�ƐF����Z
    outDiffuse.a *=
		In.Diffuse.a; //���ʌv�Z
    
    //�G�b�W�̍쐬
    //�s�N�Z���V�F�[�_�[�̍Ō�ŏ���������
    float3 eyev = In.WorldPosition.xyz - g_CameraPosition.xyz; //�����x�N�g��
    eyev = normalize(eyev); //���K��
    
    float d = dot(eyev, In.Normal.xyz);
    
    if (d >= -0.2f)
    {
        //outDiffuse.rb = 0.0f;     //�K���Ȓl�Ŗ��邳�𗎂�
        outDiffuse.g = 20.0f;
    }

}


