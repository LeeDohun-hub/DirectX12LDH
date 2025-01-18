#include "BasicShaderHeader.hlsli"
Texture2D<float4> tex : register(t0); //0번 슬롯에 지정된 텍스쳐
SamplerState smp : register(s0); //0번 슬롯에 지정된 샘플러
float4 BasicPS(Output input) : SV_TARGET
{
    return float4(tex.Sample(smp, input.uv));
}