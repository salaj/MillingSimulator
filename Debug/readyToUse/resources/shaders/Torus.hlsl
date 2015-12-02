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

//cbuffer cbLightPos : register(b3) //Vertex Shader constant buffer slot 3
//{
//	float4 lightPos[3];
//}
//
//cbuffer cbLightColors : register(b0) //Pixel Shader constant buffer slot 0
//{
//	float4 ambientColor;
//	float4 surface; //[ka, kd, ks, m]
//	float4 lightColors[3];
//}

//cbuffer cbSurfaceColor : register(b1) //Pixel Shader constant buffer slot 0
//{
//	float4 surfaceColor;
//}

struct VSInput
{
	float3 pos : POSITION;
	//float3 color : COLOR0;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	//float3 color : COLOR;
	float3 norm : NORMAL;
	//float3 viewVec : TEXCOORD0;
	//float3 lightVec0 : TEXCOORD1;
	//float3 lightVec1 : TEXCOORD2;
	//float3 lightVec2 : TEXCOORD3;
};

PSInput VS_Main(VSInput i)
{
	PSInput o = (PSInput)0;
	float4 viewPos = float4(i.pos, 1.0f);

	matrix worldView = mul(viewMatrix, worldMatrix);
	viewPos = mul(worldView, viewPos);
	//viewPos = mul(worldMatrix, viewPos);
	o.pos = mul(projMatrix, viewPos);
	return o;
}

float4 PS_Main(PSInput i) : SV_TARGET
{
	return float4(color.xyz, 1.0f);
}