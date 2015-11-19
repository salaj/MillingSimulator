/*Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);*/

cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

cbuffer color : register(b0) //Vertex Shader constant buffer slot 2
{
	float4 color;
};

struct VSInput
{
	float3 pos : POSITION;
};

struct PSInput
{
	float4 pos : SV_POSITION;
};

PSInput VS_Main(VSInput i)
{
	PSInput o = (PSInput)0;
	float4 viewPos = float4(i.pos, 1.0f);
	o.pos = viewPos;
	return o;
}

float4 PS_Main(PSInput i) : SV_TARGET
{
	return float4(color.xyz, 1.0f);
}