#include "BasicShaderHeader.hlsli"
Texture2D<float4> tex : register(t0); //0�� ���Կ� ������ �ؽ���
SamplerState smp : register(s0); //0�� ���Կ� ������ ���÷�
float4 BasicPS(Output input) : SV_TARGET
{
    return float4(tex.Sample(smp, input.uv));
}