#include "Header.hlsli"

Texture2D g_Texture : register(t0);
Texture2D g_TextureShadowDepth : register(t1);	//�V���h�E�}�b�v
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    //�����ł̐F=���ʂ̃e�N�X�`�������_�F���쐬���Ă���
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    outDiffuse *= In.Diffuse;
    
    In.ShadowPosition.xyz /= In.ShadowPosition.w;         //���e�ʂł̃s�N�Z���̍��W����
    In.ShadowPosition.x = In.ShadowPosition.x * 0.5f + 0.5f; //�e�N�X�`�����W�ɕϊ� x * ? + ?
    In.ShadowPosition.y = -In.ShadowPosition.y * 0.5f + 0.5f; //Y�͕����𔽓]���Ă��� -y * ? + ?
    
    //�V���h�E�}�b�v�e�N�X�`�����A���C�g�J��������s�N�Z���܂ł̋���(�[�x�l)���擾
    float depth = g_TextureShadowDepth.Sample(g_SamplerState, In.ShadowPosition.xy);

    //�擾�l���v�Z��̃s�N�Z���ւ̋�����菬����
    if (depth < In.ShadowPosition.z - 0.1f && In.ShadowPosition.z <= 1.0f)     //0.01��Z�t�@�C�e�B���O�␳�l
    {
        outDiffuse.rgb *= 0.5f; //�F���Â�����
    }

}


