//버텍스 세이더에서 픽셀세이더로의 주고받는 사용 구조체

struct Output
{
    float4 svpos : SV_POSITION; //시스템용 버텍스 좌표
    float2 uv : TEXCOORD; //uv치
};