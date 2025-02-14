
#include "Engine_Shader_Define.hlsli"

float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4		g_ViewMatrixInv, g_ProjMatrixInv;
float4x4		g_LightViewMatrix, g_LightProjMatrix;
texture2D		g_Texture;
texture2D		g_NormalTexture;
texture2D		g_DiffuseTexture;
texture2D		g_ShadeTexture;
texture2D		g_DepthTexture;
texture2D		g_SpecularTexture;
texture2D		g_LightDepthTexture;
texture2D		g_FinalTexture;
texture2D		g_BlurXTexture;

vector			g_vLightDir;
vector			g_vLightPos;
float			g_fLightRange;

vector			g_vLightDiffuse;
vector			g_vLightAmbient;
vector			g_vLightSpecular;

vector			g_vMtrlAmbient = 1.f;
vector			g_vMtrlSpecular = 1.f;

vector			g_vCamPosition;

struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;

	return Out;
}
struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;

};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

/* PixelShader */
PS_OUT PS_MAIN_DEBUG(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	return Out;
}

struct PS_OUT_LIGHT
{
	float4		vShade : SV_TARGET0;
	float4		vSpecular : SV_TARGET1;
};

PS_OUT_LIGHT PS_MAIN_LIGHT_DIRECTIONAL(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

	vector			vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	vector			vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float			fViewZ = vDepthDesc.y * 1000.f;

	vector			vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector			vShade = saturate(dot(normalize(g_vLightDir) * -1.f, vNormal)) + (g_vLightAmbient * g_vMtrlAmbient);

	Out.vShade = g_vLightDiffuse * saturate(vShade);

	vector			vReflect = reflect(normalize(g_vLightDir), vNormal);

	/* 현재 픽셀의 월드위치가 필요하다. */
	/* 현재 픽셀 : 화면에 그려지고 있는 디퍼드 셰이딩을 요하는 객체들(지형, 프렐이어 몬스터)을 구성하는 픽셀들 <-> 직교투영으로 그리느느 사각형 버퍼의 픽셀(x)*/
	/* 뷰포트, 투영스페이스는 화면에 그려지는 영역이 고정되어있다. */
	/* 뷰포트 상 왼쪽 위에그려지는 픽셀의 텍스쿠드 (0, 0) == 투영스페이스 상의 왼쪽위 픽셀의 위치(-1.f, 1.f) */
	/* 이 연산을 통해 투영스페이스 상의 위치를 구하낟.*/

	vector			vPosition = 0.f;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 * 투영행렬 * 1.f / w */
	vPosition.x = In.vTexcoord.x * 2.f - 1.f;
	vPosition.y = In.vTexcoord.y * -2.f + 1.f;
	vPosition.z = vDepthDesc.x;
	vPosition.w = 1.f;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vPosition *= fViewZ;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 */
	vPosition = mul(vPosition, g_ProjMatrixInv);

	/* 로컬정점위치 * 월드행렬 */
	vPosition = mul(vPosition, g_ViewMatrixInv);	

	vector			vLook = vPosition - g_vCamPosition;

	float			fSpecular = pow(saturate(dot(normalize(vLook) * -1.f, normalize(vReflect))), 30.f);

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * fSpecular;

	return Out;
}

PS_OUT_LIGHT PS_MAIN_LIGHT_POINT(PS_IN In)
{
	PS_OUT_LIGHT		Out = (PS_OUT_LIGHT)0;

	vector			vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	vector			vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float			fViewZ = vDepthDesc.y * 1000.f;

	vector			vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	/* 현재 픽셀의 월드위치가 필요하다. */
	/* 현재 픽셀 : 화면에 그려지고 있는 디퍼드 셰이딩을 요하는 객체들(지형, 프렐이어 몬스터)을 구성하는 픽셀들 <-> 직교투영으로 그리느느 사각형 버퍼의 픽셀(x)*/
	/* 뷰포트, 투영스페이스는 화면에 그려지는 영역이 고정되어있다. */
	/* 뷰포트 상 왼쪽 위에그려지는 픽셀의 텍스쿠드 (0, 0) == 투영스페이스 상의 왼쪽위 픽셀의 위치(-1.f, 1.f) */
	/* 이 연산을 통해 투영스페이스 상의 위치를 구하낟.*/

	vector			vPosition = 0.f;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 * 투영행렬 * 1.f / w */
	vPosition.x = In.vTexcoord.x * 2.f - 1.f;
	vPosition.y = In.vTexcoord.y * -2.f + 1.f;
	vPosition.z = vDepthDesc.x;
	vPosition.w = 1.f;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vPosition *= fViewZ;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 */
	vPosition = mul(vPosition, g_ProjMatrixInv);

	/* 로컬정점위치 * 월드행렬 */
	vPosition = mul(vPosition, g_ViewMatrixInv);

	vector			vLightDir = vPosition - g_vLightPos;

	float			fDistance = length(vLightDir);

	/* 빛의 감쇄값*/
	/* 픽셀과 빛이 가까우면 1에 가까운값으로 멀면 0에 가까운 값으로 범위를 벗어나면 다 0으로. */
	float			fAtt = saturate((g_fLightRange - fDistance) / g_fLightRange);

	vector			vShade = fAtt * (saturate(dot(normalize(vLightDir) * -1.f, vNormal)) + (g_vLightAmbient * g_vMtrlAmbient));

	Out.vShade = g_vLightDiffuse * saturate(vShade);

	vector			vReflect = reflect(normalize(vLightDir), vNormal);	

	vector			vLook = vPosition - g_vCamPosition;


	float			fSpecular = fAtt * pow(saturate(dot(normalize(vLook) * -1.f, normalize(vReflect))), 30.f);

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * fSpecular;

	return Out;
}

PS_OUT PS_MAIN_FINAL(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector		vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (0.f == vDiffuse.a)
		discard;


	vector		vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);

	vector		vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);

	Out.vColor = vDiffuse * vShade + vSpecular;

	vector			vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float			fViewZ = vDepthDesc.y * 1000.f;


	vector			vPosition = 0.f;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 * 투영행렬 * 1.f / w */
	vPosition.x = In.vTexcoord.x * 2.f - 1.f;
	vPosition.y = In.vTexcoord.y * -2.f + 1.f;
	vPosition.z = vDepthDesc.x;
	vPosition.w = 1.f;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vPosition *= fViewZ;

	/* 로컬정점위치 * 월드행렬 * 뷰행렬 */
	vPosition = mul(vPosition, g_ProjMatrixInv);

	/* 로컬정점위치 * 월드행렬 */
	vPosition = mul(vPosition, g_ViewMatrixInv);

	vPosition = mul(vPosition, g_LightViewMatrix);
	vPosition = mul(vPosition, g_LightProjMatrix);

	float2		vTexcoord = 0.f;

	vTexcoord.x = (vPosition.x / vPosition.w) * 0.5f + 0.5f;
	vTexcoord.y = (vPosition.y / vPosition.w) * -0.5f + 0.5f;


	float4			vLightDepth = g_LightDepthTexture.Sample(LinearSampler, vTexcoord);

	//if ((vLightDepth.x * 1000.f) < vPosition.w - 1.0f)
	//{
	//	Out.vColor = vector(Out.vColor.rgb * 0.5f, 1.f);
	//}
	return Out;
}

float		g_fWeights[13] = {
	0.0561, 0.1353, 0.278, 0.4868, 0.7261, 0.9231, 1.f, 0.9231, 0.7261, 0.4868, 0.278, 0.1353, 0.0561
};

struct PS_OUT_BLUR
{
	vector	vColor : SV_TARGET0;
};

PS_OUT_BLUR PS_MAIN_BLUR_X(PS_IN In)
{
	PS_OUT_BLUR			Out = (PS_OUT_BLUR)0;

	float2		vTexcoord = 0.f;

	for (int i = -6; i < 7; i++)
	{
		vTexcoord = float2(In.vTexcoord.x + (1.f / 1280.f) * i, In.vTexcoord.y);

		Out.vColor += g_fWeights[i + 6] * g_FinalTexture.Sample(LinearSampler_Clamp, vTexcoord);
	}

	Out.vColor /= 6.f;

	return Out;
}

PS_OUT_BLUR PS_MAIN_BLUR_Y(PS_IN In)
{
	PS_OUT_BLUR			Out = (PS_OUT_BLUR)0;

	float2		vTexcoord = 0.f;

	vector		vBlur = 0.f;

	for (int i = -6; i < 7; i++)
	{
		vTexcoord = float2(In.vTexcoord.x, In.vTexcoord.y + (1.f / 720.f) * i);

		vBlur += g_fWeights[i + 6] * g_BlurXTexture.Sample(LinearSampler_Clamp, vTexcoord);
	}

	vBlur /= 6.f;

	vector		vFinalColor = g_FinalTexture.Sample(LinearSampler, In.vTexcoord);

	Out.vColor = vFinalColor + vBlur;

	return Out;
}



technique11			DefaultTechnique
{
	/* 우리가 수행해야할 정점, 픽셀 셰이더의 진입점 함수를 지정한다. */
	pass Debug
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
	}

	pass Light_Directional
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_LIGHT_DIRECTIONAL();
	}

	pass Light_Point
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_LIGHT_POINT();
	}

	pass Final
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_FINAL();
	}

	pass BlurX
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_BLUR_X();
	}

	pass BlurY
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_BLUR_Y();
	}


}
