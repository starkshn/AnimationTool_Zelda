
#include "../../../EngineSDK/hlsl/Engine_Shader_Define.hlsli"


float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float3 g_Color = { 1.f, 0.f, 0.f };

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vColor : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float3 vColor : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    
    Out.vColor = g_Color; //In.vColor;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float3 vColor : TEXCOORD0;
};


struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

// Pixel Shader
PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = float4(g_Color, 1.f);

    return Out;
}


technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN(); // 컴파일 해라, 5.0버젼으로 VS_MAIN호출한다.
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN(); // ''
    }
}