//thsozai ==>�O��܂ł̏�ԁ@ZIP�@�@�p�X���[�h�@nabe

#include "Header.hlsli"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//�s�N�Z���̖@���𐳋K��
    float4 normal = normalize(In.Normal);
	//�����v�Z������
    float light = -dot(normal.xyz, Light.Direction.xyz);
    
    if (light < 0.6f)
        light = 0.3f;
    else if (light >= 0.6f && light <= 0.8f)
        light = 0.7f;
    else if (light > 0.8f)
        light = 0.9f;

	//�e�N�X�`���̃s�N�Z���F���擾
    outDiffuse =
		g_Texture.Sample(g_SamplerState, In.TexCoord);

    outDiffuse.rgb *=
		In.Diffuse.rgb * light; //���邳�ƐF����Z
    outDiffuse.a *=
		In.Diffuse.a; //���ʌv�Z

	//�X�y�L�����l���f�t���[�Y�Ƃ��đ�������
    //outDiffuse.rgb += specular;
    
    //�G�b�W�̍쐬
    //�s�N�Z���V�F�[�_�[�̍Ō�ŏ���������
    //float3 eyev = In.WorldPosition.xyz - g_CameraPosition.xyz;     //�����x�N�g��
    //eyev = normalize(eyev);     //���K��
    
    //float d = dot(eyev, In.Normal.xyz);
    
    //if (d >= -0.2f)
    //{
    //    //outDiffuse.rb = 0.0f;     //�K���Ȓl�Ŗ��邳�𗎂�
    //    outDiffuse.g = 20.0f;
    //}

}


