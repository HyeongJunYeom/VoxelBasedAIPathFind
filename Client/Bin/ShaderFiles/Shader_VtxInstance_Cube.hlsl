#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix	g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float	g_fVoxelSize;

const int			g_iLBB = { 1 };
const int			g_iCBB = { 1 << 1 };
const int			g_iRBB = { 1 << 2 };
const int			g_iLCB = { 1 << 3 };
const int			g_iCCB = { 1 << 4 };
const int			g_iRCB = { 1 << 5 };
const int			g_iLTB = { 1 << 6 };
const int			g_iCTB = { 1 << 7 };
const int			g_iRTB = { 1 << 8 };

const int			g_iLBC = { 1 << 9 };
const int			g_iCBC = { 1 << 10 };
const int			g_iRBC = { 1 << 11 };
const int			g_iLCC = { 1 << 12 };
const int			g_iCCC = { 1 << 13 };
const int			g_iRCC = { 1 << 14 };
const int			g_iLTC = { 1 << 15 };
const int			g_iCTC = { 1 << 16 };
const int			g_iRTC = { 1 << 17 };

const int			g_iLBF = { 1 << 18 };
const int			g_iCBF = { 1 << 19 };
const int			g_iRBF = { 1 << 20 };
const int			g_iLCF = { 1 << 21 };
const int			g_iCCF = { 1 << 22 };
const int			g_iRCF = { 1 << 23 };
const int			g_iLTF = { 1 << 24 };
const int			g_iCTF = { 1 << 25 };
const int			g_iRTF = { 1 << 26 };

struct VS_IN
{
	float3				vPosition : POSITION;

	float3				vPositionVoxel : INSTANCE_POSITION;
	float				fScaleVoxel : COLOR;
	int					iID : ID;
	int					iState : STATE;
	int					iNeighbor : NEIGHBOR;
};

struct VS_OUT
{
	float4				vPosition : POSITION;
	float				fScaleVoxel : COLOR;
	int					iID : ID;
	int					iState : STATE;
	int					iNeighbor : NEIGHBOR;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	vector		vPosition = (vector(In.vPosition, 1.f) + vector(In.vPositionVoxel, 0.f));
	Out.vPosition = mul(vPosition, g_WorldMatrix);

	Out.fScaleVoxel = In.fScaleVoxel * 0.5f;
	Out.iID = In.iID;
	Out.iState = In.iState;
	Out.iNeighbor = In.iNeighbor;

	return Out;
}

struct GS_IN
{
	float4		vPosition : POSITION;
	float		fScaleVoxel : COLOR;
	int			iID : ID;
	int			iState : STATE;
	int			iNeighbor : NEIGHBOR;
};

struct GS_OUT
{
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float4		vWorldPos : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
	int			iID : ID;
	int			iState : STATE;
	float		fAlpha : ALPHA;
};

///* 정점을 생성한다. */
[maxvertexcount(36)]
//	[maxvertexcount(36)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
	GS_OUT		Out[8] = {
		(GS_OUT)0.f,
		(GS_OUT)0.f,
		(GS_OUT)0.f,
		(GS_OUT)0.f,
		(GS_OUT)0.f,
		(GS_OUT)0.f,
		(GS_OUT)0.f,
		(GS_OUT)0.f,
	};

	float		fVoxelSizeHalf = { g_fVoxelSize / 2.f };
	float3		vRight =	float3(In[0].fScaleVoxel, 0.f, 0.f);
	float3		vUp =		float3(0.f, In[0].fScaleVoxel, 0.f);
	float3		vLook =		float3(0.f, 0.f, In[0].fScaleVoxel);

	matrix		matVP = mul(g_ViewMatrix, g_ProjMatrix);

	for (int i = 0; i < 8; ++i)
	{
		Out[i].fAlpha = clamp(0.5f * i, 0.f, 1.f);
		Out[i].iID = In[0].iID;
		Out[i].iState = In[0].iState;
	}

	Out[0].vPosition = vector(In[0].vPosition.xyz - vRight + vUp - vLook, 1.f);
	Out[0].vWorldPos = Out[0].vPosition;
	Out[0].vNormal = vector(-vRight.x, vUp.y, -vLook.z, 0.f);
	Out[0].vPosition = mul(Out[0].vPosition, matVP);
	Out[0].vProjPos = Out[0].vPosition;
	Out[0].vNormal = normalize(mul(Out[0].vNormal, matVP));

	Out[1].vPosition = vector(In[0].vPosition.xyz + vRight + vUp - vLook, 1.f);
	Out[1].vWorldPos = Out[1].vPosition;
	Out[1].vNormal = vector(vRight.x, vUp.y, -vLook.z, 0.f);
	Out[1].vPosition = mul(Out[1].vPosition, matVP);
	Out[1].vProjPos = Out[1].vPosition;
	Out[1].vNormal = normalize(mul(Out[1].vNormal, matVP));

	Out[2].vPosition = vector(In[0].vPosition.xyz + vRight - vUp - vLook, 1.f);
	Out[2].vWorldPos = Out[2].vPosition;
	Out[2].vNormal = vector(vRight.x, -vUp.y, -vLook.z, 0.f);
	Out[2].vPosition = mul(Out[2].vPosition, matVP);
	Out[2].vProjPos = Out[2].vPosition;
	Out[2].vNormal = normalize(mul(Out[2].vNormal, matVP));

	Out[3].vPosition = vector(In[0].vPosition.xyz - vRight - vUp - vLook, 1.f);
	Out[3].vWorldPos = Out[3].vPosition;
	Out[3].vNormal = vector(-vRight.x, -vUp.y, -vLook.z, 0.f);
	Out[3].vPosition = mul(Out[3].vPosition, matVP);
	Out[3].vProjPos = Out[3].vPosition;
	Out[3].vNormal = normalize(mul(Out[3].vNormal, matVP));

	Out[4].vPosition = vector(In[0].vPosition.xyz - vRight + vUp + vLook, 1.f);
	Out[4].vWorldPos = Out[4].vPosition;
	Out[4].vNormal = vector(-vRight.x, vUp.y, vLook.z, 0.f);
	Out[4].vPosition = mul(Out[4].vPosition, matVP);
	Out[4].vProjPos = Out[4].vPosition;
	Out[4].vNormal = normalize(mul(Out[4].vNormal, matVP));

	Out[5].vPosition = vector(In[0].vPosition.xyz + vRight + vUp + vLook, 1.f);
	Out[5].vWorldPos = Out[5].vPosition;
	Out[5].vNormal = vector(vRight.x, vUp.y, vLook.z, 0.f);
	Out[5].vPosition = mul(Out[5].vPosition, matVP);
	Out[5].vProjPos = Out[5].vPosition;
	Out[5].vNormal = normalize(mul(Out[5].vNormal, matVP));

	Out[6].vPosition = vector(In[0].vPosition.xyz + vRight - vUp + vLook, 1.f);
	Out[6].vWorldPos = Out[6].vPosition;
	Out[6].vNormal = vector(vRight.x, vUp.y, vLook.z, 0.f);
	Out[6].vPosition = mul(Out[6].vPosition, matVP);
	Out[6].vProjPos = Out[6].vPosition;
	Out[6].vNormal = normalize(mul(Out[6].vNormal, matVP));

	Out[7].vPosition = vector(In[0].vPosition.xyz - vRight - vUp + vLook, 1.f);
	Out[7].vWorldPos = Out[7].vPosition;
	Out[7].vNormal = vector(-vRight.x, -vUp.y, vLook.z, 0.f);
	Out[7].vPosition = mul(Out[7].vPosition, matVP);
	Out[7].vProjPos = Out[7].vPosition;
	Out[7].vNormal = normalize(mul(Out[7].vNormal, matVP));


	if (0 == (g_iCCB & In[0].iNeighbor))
	{
		//	후면 Z-
		Vertices.Append(Out[0]);
		Vertices.Append(Out[1]);
		Vertices.Append(Out[2]);

		Vertices.RestartStrip();

		Vertices.Append(Out[0]);
		Vertices.Append(Out[2]);
		Vertices.Append(Out[3]);

		Vertices.RestartStrip();
	}

	if (0 == (g_iCCF & In[0].iNeighbor))
	{
		//	정면 Z+
		Vertices.Append(Out[5]);
		Vertices.Append(Out[4]);
		Vertices.Append(Out[7]);

		Vertices.RestartStrip();

		Vertices.Append(Out[5]);
		Vertices.Append(Out[7]);
		Vertices.Append(Out[6]);

		Vertices.RestartStrip();
	}

	if (0 == (g_iRCC & In[0].iNeighbor))
	{
		//	우측면 X+
		Vertices.Append(Out[1]);
		Vertices.Append(Out[5]);
		Vertices.Append(Out[6]);

		Vertices.RestartStrip();

		Vertices.Append(Out[1]);
		Vertices.Append(Out[6]);
		Vertices.Append(Out[2]);


		Vertices.RestartStrip();
	}
	
	if (0 == (g_iLCC & In[0].iNeighbor))
	{
		//	좌측면 X-
		Vertices.Append(Out[4]);
		Vertices.Append(Out[0]);
		Vertices.Append(Out[3]);

		Vertices.RestartStrip();

		Vertices.Append(Out[4]);
		Vertices.Append(Out[3]);
		Vertices.Append(Out[7]);

		Vertices.RestartStrip();
	}

	if (0 == (g_iCTC & In[0].iNeighbor))
	{
		//	상부 Y+
		Vertices.Append(Out[4]);
		Vertices.Append(Out[5]);
		Vertices.Append(Out[1]);

		Vertices.RestartStrip();

		Vertices.Append(Out[4]);
		Vertices.Append(Out[1]);
		Vertices.Append(Out[0]);

		Vertices.RestartStrip();
	}

	if (0 == (g_iCBC & In[0].iNeighbor))
	{
		//	하부 Y-
		Vertices.Append(Out[3]);
		Vertices.Append(Out[2]);
		Vertices.Append(Out[6]);

		Vertices.RestartStrip();

		Vertices.Append(Out[3]);
		Vertices.Append(Out[6]);
		Vertices.Append(Out[7]);

		Vertices.RestartStrip();
	}
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float4		vWorldPos : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
	int			iID : ID;
	int			iState : STATE;
	float		fAlpha : ALPHA;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	float4 vColor;

    switch (In.iID)
    {
		case 0:		//	Floor
			vColor = vector(0.f, 1.f, 0.f, In.fAlpha);
            break;
		case 1:		//	Wall
			vColor = vector(1.f, 1.f, 0.f, In.fAlpha);
            break;
		case 2:		//	Obstacle
			vColor = vector(0.f, 1.f, 1.f, In.fAlpha);
            break;
    }

	Out.vNormal = vector((In.vNormal * 0.5f + 0.5f).xyz, 0.f);

	switch (In.iState)
	{
	case 1:		//	Add Green
		vColor = float4(1.f, 1.f, 1.f, 1.f);
		break;

	case 9:		// NEIGHBOR
		vColor = float4(1.f, 0.f, 0.f, In.fAlpha);
		break;

	case 6:		//	White	//	Start
	case 7:		//	White	//	Goal
		vColor = float4(1.f, 1.f, 1.f, In.fAlpha);
		break;

	case 3:		//	Red		//	Close
		vColor = float4(1.f, 0.f, 0.f, In.fAlpha);
		break;

	case 2:		// Violet //	Open
		vColor = float4(1.f, 0.f, 1.f, In.fAlpha);
		break;

	case 4:		// Blue		//	Best
		vColor = float4(0.f, 0.f, 1.f, In.fAlpha);
		break;

	case 10:	//	Reserve
		vColor = float4(1.f, 0.f, 0.f, In.fAlpha);
		break;
	}

	vColor.a = 0.6f;

	Out.vColor = vColor;
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
	
	return Out;
}

technique11 DefaultTechnique
{
	pass Default
	{
		//	SetRasterizerState(RS_Wireframe);sa                                                                             
		SetRasterizerState(RS_Default);
        //	SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		//	SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}