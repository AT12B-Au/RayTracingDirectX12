//�s�N�Z���V�F�[�_�[
#include "Header.hlsli" //�K���C���N���[�h
//
//in �͓��͂���Ă���f�[�^�[
//out �͏o�͂���f�[�^
//

//DirectX�̃e�N�X�`���̐ݒ���󂯌p��
Texture2D g_Texture : register(t0);//�e�N�X�`���[�O��
Texture2D g_TextureNormal : register(t1); //�e�N�X�`���[�P��
//DirectX�̃T���v���[�X�e�[�g�ݒ���󂯌p��
SamplerState g_SamplerState : register(s1);//�e�N�X�`���T���v

void main(in PS_IN In, out float4 outDiffuse : SV_Target0)
{
    float4 normalMap = g_TextureNormal.Sample(g_SamplerState, In.TexCoord);
    
    normalMap = normalMap * 2.0f - 1.0f;
    
    float4 normal;
    normal.x = -normalMap.x;
    normal.y = normalMap.z;
    normal.z = normalMap.y;
    normal.w = 0.0f;
    
   //�s�N�Z���̖@���𐳋K��
    normal = normalize(normal);
	//�����v�Z������
    float light = -dot(normal.xyz, Light.Direction.xyz);

	//�X�y�L�����̌v�Z
	//�J��������s�N�Z���֌������x�N�g��
    float3 eyev = In.WorldPosition.xyz - g_CameraPosition.xyz;
    eyev = normalize(eyev); //���K������

	////���̔��˃x�N�g�����v�Z
    //float3 refv = reflect(Light.Direction.xyz, normal.xyz);
    //refv = normalize(refv); //���K������

	//�n�[�t�x�N�g���쐬
    //float3 halfv = eyev + Light.Direction.xyz;
    //halfv = normalize(halfv); //���K������

	//���ʔ��˂̌v�Z
    float specular = -dot(eyev, normal.xyz);
    specular = saturate(specular); //�l���T�`�����[�g
    specular = pow(specular, 30); //�����ł͂R�O�悵�Ă݂�

	//�e�N�X�`���̃s�N�Z���F���擾
    outDiffuse =
		g_Texture.Sample(g_SamplerState, In.TexCoord);

    outDiffuse.rgb *=
		In.Diffuse.rgb * light; //���邳�ƐF����Z
    outDiffuse.a *=
		In.Diffuse.a; //���ʌv�Z

	//�X�y�L�����l���f�t���[�Y�Ƃ��đ�������
    outDiffuse.rgb += specular;
}