#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix	g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4	g_vColor = float4(0.f, 0.f, 0.f, 0.f);

struct VS_IN
{
	float3				vPosition : POSITION;

	float3				vStart : POSITION_START;
	float3				vEnd : POSITION_END;
};

struct VS_OUT
{
	float4				vStart : POSITION_START;
	float4				vEnd : POSITION_END;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	Out.vStart = vector(In.vStart, 1.f);
	Out.vEnd = vector(In.vEnd, 1.f);

	return Out;
}

struct GS_IN
{
	float4				vStart : POSITION_START;
	float4				vEnd : POSITION_END;
};

struct GS_OUT
{
	float4				vPosition : SV_POSITION;
};

///* 정점을 생성한다. */
[maxvertexcount(2)]
//	[maxvertexcount(36)]
void GS_MAIN(point GS_IN In[1], inout LineStream<GS_OUT> Vertices)
{
	GS_OUT		Out[2] = {
		(GS_OUT)0.f,
		(GS_OUT)0.f,
	};

	float4x4		MatVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out[0].vPosition = mul(vector(In[0].vStart), MatVP);
	Out[1].vPosition = mul(vector(In[0].vEnd), MatVP);

	Vertices.Append(Out[0]);
	Vertices.Append(Out[1]);
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	//	Out.vColor = g_vColor;
	Out.vColor = vector(1.f, 1.f, 1.f, 1.f);

	return Out;
}

technique11 DefaultTechnique
{
	pass Default
	{
		//	SetRasterizerState(RS_Wireframe);sa                                                                             
		SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        //	SetDepthStencilState(DSS_Default, 0);
        //	SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}