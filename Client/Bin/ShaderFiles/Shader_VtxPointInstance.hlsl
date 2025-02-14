
#include "../../../EngineSDK/hlsl/Engine_Shader_Define.hlsli"

/* 상수테이블 ConstantTable */
float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D		g_Texture;

vector			g_vCamPosition;


struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vPSize : PSIZE;	

	row_major float4x4	TransformMatrix : WORLD;
	float2		vLifeTime : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;	
	float2		vLifeTime : TEXCOORD0;
};

/* 1. 정점위치에 대한 기초적인 변환과정을 수행한다.(월드변환, 뷰변환, ) */

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	matrix			matWV, matWVP;	

	float			fScaleX = length(In.TransformMatrix._11_12_13);
	float			fScaleY = length(In.TransformMatrix._21_22_23);
	float			fScaleZ = length(In.TransformMatrix._31_32_33);
		
	matrix			TransformMatrix = float4x4(float4(1.f, 0.f, 0.f, 0.f), float4(0.f, 1.f, 0.f, 0.f), float4(0.f, 0.f, 1.f, 0.f), In.TransformMatrix._41_42_43_44);
	vector			vPosition = mul(float4(In.vPosition, 1.f), TransformMatrix);
		
	Out.vPosition = mul(vPosition, g_WorldMatrix);
	Out.vPSize = float2(In.vPSize.x * fScaleX, In.vPSize.y * fScaleY);	
	Out.vLifeTime = In.vLifeTime;

	return Out;
}

/* VS : 정점하나 당 호출되는 함수다. */
/* GS : 그리고자하는 도형(POINT, LINE, TRIANGLE) 하나당 호출되는 함수이다. */
/* PS : 픽셀하나 당 호출되는 함수다. */
struct GS_IN
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;
	float2		vLifeTime : TEXCOORD0;
};

struct GS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	float2		vLifeTime : TEXCOORD1;
};

//GS_MAIN(triangle GS_IN In[3])
//GS_MAIN(line GS_IN In[2])

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> OutStream)
{
	GS_OUT			Out[4];
	vector			vLook = g_vCamPosition - In[0].vPosition;
	float3			vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook.xyz)) * In[0].vPSize.x * 0.5f;
	float3			vUp = normalize(cross(vLook.xyz, vRight)) * In[0].vPSize.y * 0.5f;

	matrix			matVP = mul(g_ViewMatrix, g_ProjMatrix);
	
	Out[0].vPosition = float4(In[0].vPosition.xyz + vRight + vUp, 1.f);
	Out[0].vPosition = mul(Out[0].vPosition, matVP);
	Out[0].vTexcoord = float2(0.f, 0.f);
	Out[0].vLifeTime = In[0].vLifeTime;

	Out[1].vPosition = float4(In[0].vPosition.xyz - vRight + vUp, 1.f);
	Out[1].vPosition = mul(Out[1].vPosition, matVP);
	Out[1].vTexcoord = float2(1.f, 0.f);
	Out[1].vLifeTime = In[0].vLifeTime;

	Out[2].vPosition = float4(In[0].vPosition.xyz - vRight - vUp, 1.f);
	Out[2].vPosition = mul(Out[2].vPosition, matVP);
	Out[2].vTexcoord = float2(1.f, 1.f);
	Out[2].vLifeTime = In[0].vLifeTime;

	Out[3].vPosition = float4(In[0].vPosition.xyz + vRight - vUp, 1.f);
	Out[3].vPosition = mul(Out[3].vPosition, matVP);
	Out[3].vTexcoord = float2(0.f, 1.f);
	Out[3].vLifeTime = In[0].vLifeTime;

	OutStream.Append(Out[0]);
	OutStream.Append(Out[1]);
	OutStream.Append(Out[2]);
	OutStream.RestartStrip();

	OutStream.Append(Out[0]);
	OutStream.Append(Out[2]);
	OutStream.Append(Out[3]);
	OutStream.RestartStrip();			
}





/* 정점의 위치벡터를 위치벡터의 w로 나눈다. */
/* 뷰포트 영역으로 정점들을 변환한다. */
/* 래스터라이즈(정점 정보를 보간하여 픽셀의 정보가 생성된다.) */
/* 이렇게 픽셀을 PS_MAIN() 호출하면서 전달해준다. */

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	float2		vLifeTime : TEXCOORD1;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

/* PixelShader */
PS_OUT PS_MAIN_LOOP(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

		
	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);	

	if (Out.vColor.a < 0.3f)
		discard;

	Out.vColor.rgb = 1.f - saturate(In.vLifeTime.x - In.vLifeTime.y);


	return Out;
}

PS_OUT PS_MAIN_ONCE(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;


	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	if (Out.vColor.a < 0.3f)
		discard;

	Out.vColor.gb = 0.f;

	Out.vColor.a = saturate(In.vLifeTime.x - In.vLifeTime.y) * 2.f;

	Out.vColor.r = 1.f - Out.vColor.a;

	if (In.vLifeTime.x <= In.vLifeTime.y)
		discard;

	return Out;
}

technique11			DefaultTechnique
{
	/* 우리가 수행해야할 정점, 픽셀 셰이더의 진입점 함수를 지정한다. */
	pass Loop
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_LOOP();
	}

	pass Once
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN_ONCE();
	}
}
