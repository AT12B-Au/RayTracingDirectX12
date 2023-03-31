//�s�N�Z���V�F�[�_�[
#include "Header.hlsli" //�K���C���N���[�h
//
//in �͓��͂���Ă���f�[�^�[
//out �͏o�͂���f�[�^
//

//DirectX�̃e�N�X�`���̐ݒ���󂯌p��
Texture2D g_Texture : register(t0); //�e�N�X�`���[
Texture2D g_TextureNormal : register(t1); //�m�[�}���}�b�v
Texture2D g_TextureShadowDepth : register(t2); //�V���h�E
//DirectX�̃T���v���[�X�e�[�g�ݒ���󂯌p��
SamplerState g_SamplerState : register(s1); //�e�N�X�`���T���v

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
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
    
    In.ShadowPosition.xyz /= In.ShadowPosition.w; //���e�ʂł̃s�N�Z���̍��W����
    In.ShadowPosition.x = In.ShadowPosition.x * 0.5f + 0.5f; //�e�N�X�`�����W�ɕϊ� x * ? + ?
    In.ShadowPosition.y = -In.ShadowPosition.y * 0.5f + 0.5f; //Y�͕����𔽓]���Ă��� -y * ? + ?
    
    //�V���h�E�}�b�v�e�N�X�`�����A���C�g�J��������s�N�Z���܂ł̋���(�[�x�l)���擾
    float depth = g_TextureShadowDepth.Sample(g_SamplerState, In.ShadowPosition.xy);
    
    //�擾�l���v�Z��̃s�N�Z���ւ̋�����菬����
    if (depth < In.ShadowPosition.z - 0.01f)     //0.01��Z�t�@�C�e�B���O�␳�l
    {
        outDiffuse.rgb *= 0.5f; //�F���Â�����
    }
}