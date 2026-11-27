
#include "../../../EngineSDK/hlsl/Engine_Shader_Define.hlsli"

float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;


struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vTexcoord : TEXCOORD0;	
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float3		vTexcoord : TEXCOORD0;
};


VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT			Out = (VS_OUT)0;

	matrix			matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vPosition.z = Out.vPosition.w;
	Out.vTexcoord = In.vTexcoord;

	return Out;
}


struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float3		vTexcoord : TEXCOORD0;

};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;	

};

/* PixelShader */
PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

    float3 ray = normalize(In.vTexcoord);
    float3 camera = g_WorldMatrix[3].xyz;
    Out.vColor = float4(0.19f, 0.21f, 0.23f, 1.f);
    // A visual ground reference only. The original IK collision objects are separate.
    if (ray.y < -0.0001f && camera.y > 0.f)
    {
        float distanceToGround = -camera.y / ray.y;
        float2 ground = (camera + ray * distanceToGround).xz;
        float2 width = max(fwidth(ground), 0.0001f);
        float2 edge = abs(frac(ground - 0.5f) - 0.5f) / width;
        float gridLine = 1.f - saturate(min(edge.x, edge.y));
        float fade = 1.f - smoothstep(15.f, 60.f, distanceToGround);
        float3 floorColor = lerp(float3(0.24f, 0.28f, 0.31f), float3(0.38f, 0.44f, 0.47f), gridLine * fade);
        Out.vColor.rgb = lerp(Out.vColor.rgb, floorColor, 1.f - smoothstep(60.f, 120.f, distanceToGround));
    }
    Out.vColor.a = 1.f;

	return Out;
}


technique11			DefaultTechnique
{
	/* 우리가 수행해야할 정점, 픽셀 셰이더의 진입점 함수를 지정한다. */
	pass DefaultPass
	{
		SetRasterizerState(RS_Cull_Front);
		SetDepthStencilState(DSS_None, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}
