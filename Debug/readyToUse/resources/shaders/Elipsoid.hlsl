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

cbuffer cbLightPos : register(b3) //Vertex Shader constant buffer slot 3
{
	float4 lightPos[3];
}

cbuffer cbLightColors : register(b0) //Pixel Shader constant buffer slot 0
{
	float4 ambientColor;
	float4 surface; //[ka, kd, ks, m]
	float4 lightColors[3];
}

cbuffer cbSurfaceColor : register(b1) //Pixel Shader constant buffer slot 0
{
	float4 surfaceColor;
}

cbuffer lightIntensity : register(b2) //Pixel Shader constant buffer slot 0
{
	float4 intensity;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};
struct PSInput {
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float3 viewVec : TEXCOORD0;
};
PSInput VS_Main(VSInput i)
{
	PSInput o = (PSInput)0;
	o.pos = float4(i.pos.x / 400, i.pos.y / 400, 0.0f, 1.0f);
	o.norm = i.norm.xyz;
	return o;
}
float4 PS_Main(PSInput i) : SV_TARGET
{
	float3 viewVec = normalize(float3(0.0f, 3.0f, 10.0f));
	float3 normal = normalize(i.norm);
	float3 color = float3(1.0f, 1.0f, 0.0f);
	float3 vn = dot(viewVec, normal);
	vn = clamp(vn, 0.0f, 1.0f);
	float3 vm = vn;
	vn = pow(vn, 1.0f);
	color *= vn;
	vm = pow(vm, intensity.x);
	return float4(color, 1);
}
