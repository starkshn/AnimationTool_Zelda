
#include "../../../EngineSDK/hlsl/Engine_Shader_Define.hlsli"

float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D		g_DiffuseTexture[2];
Texture2D		g_MaskTexture;
Texture2D		g_BrushTexture;

vector			g_vBrushPos = float4(20.f, 0.f, 20.f, 1.f);
float			g_fBrushRange = 10.f;


struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;	
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

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
	Out.vTexcoord = In.vTexcoord;
	Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
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

	vector			vSourDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord * 50.f);	
	vector			vDestDiffuse = g_DiffuseTexture[1].Sample(LinearSampler, In.vTexcoord * 50.f);
	vector			vMask = g_MaskTexture.Sample(LinearSampler, In.vTexcoord);
	vector			vBrush = /**/(vector)0.f;

	if (g_vBrushPos.x - g_fBrushRange <= In.vWorldPos.x && In.vWorldPos.x < g_vBrushPos.x + g_fBrushRange && 
		g_vBrushPos.z - g_fBrushRange <= In.vWorldPos.z && In.vWorldPos.z < g_vBrushPos.z + g_fBrushRange)
	{
		float2		vTexcoord = 0.f;
		
		vTexcoord.x = (In.vWorldPos.x - (g_vBrushPos.x - g_fBrushRange)) / (2.f * g_fBrushRange);
		vTexcoord.y = ((g_vBrushPos.z + g_fBrushRange) - In.vWorldPos.z) / (2.f * g_fBrushRange);

		vBrush = g_BrushTexture.Sample(LinearSampler, vTexcoord);
	}

	// Out.vDiffuse = vDestDiffuse * vMask + vSourDiffuse * (1.f - vMask) + vBrush;
	Out.vDiffuse = 1.f;
	Out.vDiffuse.a = 1.f;

	/* -1.f ~ 1.f -> 0.f ~ 1.f */
	Out.vNormal = float4(In.vNormal.xyz * 0.5f + 0.5f, 0.f);

	Out.vDepth = float4(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
	Out.vWorld = In.vWorldPos;
	return Out;
}

struct PS_OUT_HEIGHT
{
	float		fHeight : SV_TARGET0;
};

PS_OUT_HEIGHT PS_MAIN_HEIGHT(PS_IN In)
{
	PS_OUT_HEIGHT			Out = (PS_OUT_HEIGHT)0;

	Out.fHeight = In.vWorldPos.y;
	
	return Out;
}

technique11			DefaultTechnique
{
	
	pass DefaultPass
	{
		SetRasterizerState(RS_Wireframe);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass Height
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_HEIGHT();
	}


}
