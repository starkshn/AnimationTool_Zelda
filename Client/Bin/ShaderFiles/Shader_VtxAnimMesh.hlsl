
#include "../../../EngineSDK/hlsl/Engine_Shader_Define.hlsli"
float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

/* 메시에 영향을 주는 정점들. */
float4x4		g_BoneMatrices[512];
Texture2D		g_DiffuseTexture;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;	
	float3		vTangent : TANGENT;
	uint4		vBlendIndices : BLENDINDEX;
	float4		vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;
};

/* 1. 정점위치에 대한 기초적인 변환과정을 수행한다.(월드변환, 뷰변환, ) */

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	float			fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);

	matrix			BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x + 
		g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y + 
		g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z + 
		g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

	vector		vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
	vector		vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);

	Out.vPosition = mul(vPosition, matWVP);
	Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
	Out.vTexcoord = In.vTexcoord;
	Out.vWorldPos = mul(vPosition, g_WorldMatrix);
	Out.vProjPos = Out.vPosition;
	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;
};

struct PS_OUT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;
	float4		vWorld : SV_TARGET3;
};

/* PixelShader */
PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;

	/* -1.f ~ 1.f -> 0.f ~ 1.f */
	Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
	Out.vWorld = In.vWorldPos;
	return Out;
}

struct PS_OUT_NONPICKED
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;	
};

/* PixelShader */
PS_OUT_NONPICKED PS_MAIN_NONPICKED(PS_IN In)
{
	PS_OUT_NONPICKED			Out = (PS_OUT_NONPICKED)0;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    vMtrlDiffuse = vector(0.5f, 0.5f, 0.5f, 1.f);
	if (vMtrlDiffuse.a < 0.1f)
		discard;

	Out.vDiffuse = vMtrlDiffuse;

	/* -1.f ~ 1.f -> 0.f ~ 1.f */
	Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
	
	return Out;
}

struct PS_OUT_LIGHTDEPTH
{
	float4		vLightDepth : SV_TARGET0;
};


PS_OUT_LIGHTDEPTH PS_MAIN_LIGHTDEPTH(PS_IN In)
{
	PS_OUT_LIGHTDEPTH			Out = (PS_OUT_LIGHTDEPTH)0;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (vMtrlDiffuse.a < 0.1f)
		discard;
	
	Out.vLightDepth = float4(In.vProjPos.w / 1000.f, 0.f, 0.f, 0.f);

	return Out;
}

technique11			DefaultTechnique
{
	
	pass DefaultPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass NonPicked
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_NONPICKED();
	}

	pass LightDepth
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_LIGHTDEPTH();
	}



}
