#include "BasicShaderHeader.hlsli"
Output BasicVS( float4 pos : POSITION, float2 uv : TEXCOORD ) 
{
    Output output; //�ȼ� ���̴��� �ѱ�� ��ġ
    output.svpos = pos; 
    output.uv = uv;
	return output;
}