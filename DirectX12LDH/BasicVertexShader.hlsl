#include "BasicShaderHeader.hlsli"
Output BasicVS( float4 pos : POSITION, float2 uv : TEXCOORD ) 
{
    Output output; //픽셀 세이더에 넘기는 수치
    output.svpos = pos; 
    output.uv = uv;
	return output;
}