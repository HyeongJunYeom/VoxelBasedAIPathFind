
#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4 g_vColor;

textureCUBE	g_Texture;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition_World : POSITION;
	float4		vPosition : SV_POSITION;
	float3		vTexcoord : TEXCOORD0;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);	

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;
	Out.vPosition_World = mul(vector(In.vPosition, 1.f), g_WorldMatrix);

	return Out;
}

struct PS_IN
{
	float4		vPosition_World : POSITION;
	float4		vPosition : SV_POSITION;
	float3		vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
	
	return Out;
}

PS_OUT PS_COLOR_GREEN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
    Out.vColor = float4(0.f, 1.f, 0.f, 1.f);

	return Out;
}

PS_OUT PS_COLOR_YELLOW(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
	Out.vColor = float4(1.f, 1.f, 0.f, 1.f);

	return Out;
}

PS_OUT PS_COLOR_CUSTOM(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
	Out.vColor = g_vColor;

	return Out;
}


technique11 DefaultTechnique
{
	pass Default
	{
		SetRasterizerState(RS_Sky);
		SetDepthStencilState(DSS_Sky, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass WireFrame_Floor
	{
		SetRasterizerState(RS_Wireframe);
		//	SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_COLOR_CUSTOM();
	}

	pass WireFrame_Wall
	{
		SetRasterizerState(RS_Wireframe);
		//	SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_COLOR_CUSTOM();
	}

	pass Color_Floor
	{
		SetRasterizerState(RS_Default);
		//	SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_COLOR_CUSTOM();
	}

	pass Color_Wall
	{
		SetRasterizerState(RS_Default);
		//	SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_COLOR_CUSTOM();
	}
}