//�s�N�Z���V�F�[�_�[
#include "Header.hlsli" //�K���C���N���[�h
//
//in �͓��͂���Ă���f�[�^�[
//out �͏o�͂���f�[�^
//

//DirectX�̃e�N�X�`���̐ݒ���󂯌p��
Texture2D g_Texture : register(t0);//�e�N�X�`���[�O��
//DirectX�̃T���v���[�X�e�[�g�ݒ���󂯌p��
SamplerState g_SamplerState : register(s0);//�e�N�X�`���T���v

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//���̃s�N�Z���Ɏg����e�N�X�`���̐F���擾
	outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
	//���͂��ꂽ�s�N�Z���̐F�����̂܂܏o��
	outDiffuse *= In.Diffuse;

	////�O���[�X�P�[���ɂ��Ă݂�
	//outDiffuse.rgb = 
	//	  outDiffuse.r * 0.3
	//	+ outDiffuse.g * 0.6
	//	+ outDiffuse.b * 0.1;

	//////�Z�s�A���ϊ����Ă݂�
	//float4 sepia = float4(0.960, 0.784, 0.580, 1);
	//outDiffuse *= sepia;

	////outDiffuse = pow(outDiffuse,6);
	
	
	//2022_11_07�ǉ�	//������F�Ƃ���
    //float2 len = float2(0.5f, 0.5f);		//�e�N�X�`���̒��S���W
    //len -= In.TexCoord.xy;					//���S���W����e�N�X�`�����W������
    //len = length(len);						//�x�N�g���̒������v�Z
	
    //outDiffuse.rgb = len.x;					//�������s�N�Z����RGB�֏o��

	//2022_11_07�ǉ�	//�F�̔��]�Ł@�^�񒆂����邢
    //float2 len = float2(0.5f, 0.5f); //�e�N�X�`���̒��S���W
    //len -= In.TexCoord.xy; //���S���W����e�N�X�`�����W������
    //len = 1.0f - length(len); //�x�N�g���̒������v�Z
	
    //outDiffuse.rgb = pow(len.x, 4); //�������s�N�Z����RGB�֏o��
	
	//2022_11_07�ǉ�	//�~�`�E�B���h�}�X�N
    //float2 len = float2(0.5f, 0.5f); //�e�N�X�`���̒��S���W
    //len -= In.TexCoord.xy; //���S���W����e�N�X�`�����W������
    //len = length(len); //�x�N�g���̒������v�Z
    //if (len.x < 0.3f)
    //{
    //    outDiffuse.rgb *= 1.0f;
    //}
    //else
    //{
    //    outDiffuse.rgb = 0.0f;
    //}
	
	////2022_11_07�ǉ�	//RGB�V�t�g
 //   float4 colbuf = 0;
 //   float offset = (1.0f / 650.0f) * Parameter.x; //650 �e�N�X�`���̃T�C�Y	//Parameter.x���\���s�N�Z�����̃e�N�X�`�����W

	////R�v�f���擾(offset�����ꂽ�ꏊ����)
 //   colbuf.r = g_Texture.Sample(g_SamplerState, float2(In.TexCoord.x - offset, In.TexCoord.y)).r;
	////G�v�f���擾
 //   colbuf.g = g_Texture.Sample(g_SamplerState, In.TexCoord).g;
	////B�v�f���擾
 //   colbuf.b = g_Texture.Sample(g_SamplerState, float2(In.TexCoord.x + offset, In.TexCoord.y)).b;
	
 //   outDiffuse.rgb = colbuf.rgb;
	
	outDiffuse.a = 1.0f;
}