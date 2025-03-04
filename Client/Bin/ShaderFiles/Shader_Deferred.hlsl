#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_LightViewMatrix, g_LightProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;


texture2D	g_Texture;
texture2D	g_NormalTexture;	
texture2D	g_DiffuseTexture;
texture2D	g_ShadeTexture;
texture2D	g_DepthTexture;
texture2D	g_SpecularTexture;
texture2D	g_LightDepthTexture;
texture2D	g_SSAOTexture;

float4		g_vLightDir;
float4		g_vLightPos;
float		g_fLightRange;

float4		g_vLightDiffuse;
float4		g_vLightAmbient;
float4		g_vLightSpecular;

float4		g_vMtrlAmbient = float4(1.f, 1.f, 1.f, 1.f);
float4		g_vMtrlSpecular = float4(1.f, 1.f, 1.f, 1.f);

float4		g_vCamPosition;

/* For.SSAO */
float3 g_vRandomRay[16] =
{
    float3(0.2024537f, 0.841204f, -0.9060141f),
    float3(-0.2200423f, 0.6282339f, -0.8275437f),
    float3(0.3677659f, 0.1086345f, -0.4466777),
    float3(0.8775856f, 0.4617546f, -0.6427765f),
    
    float3(0.7867433f, -0.141479f, -0.1567597f),
    float3(0.4839356f, -0.8253108f, -0.1563844f),
    float3(0.4401554f, -0.4228428f, -0.3300118f),
    float3(0.0019193f, -0.8048455f, 0.0726584f),
    
    float3(-0.7578573f, -0.5583301f, 0.2347527f),
    float3(-0.4540417f, -0.252365f, 0.0694318f),
    float3(-0.0483353f, -0.2527294f, 0.5924745f),
    float3(-0.4192392f, 0.2084218f, -0.3672943f),
    
    float3(-0.8433938f, 0.141271f, 0.2202872f),
    float3(-0.4037157f, -0.8263387f, 0.4698132f),
    float3(-0.6657394f, 0.6298575f, 0.6342437f),
    float3(-0.0001783f, 0.2834622f, 0.8343929f)
};

struct VS_IN
{
	float3		vPosition : POSITION;	
	float2		vTexcoord : TEXCOORD;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;	
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
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

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD;
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

struct PS_OUT_LIGHT
{
	float4		vShade : SV_TARGET0;
	float4		vSpecular : SV_TARGET1;
};


/* 빛 하나당 480000 수행되는 쉐이더. */

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
	PS_OUT_LIGHT			Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	float4		vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.0f;
	
	Out.vShade = g_vLightDiffuse * saturate(max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + g_vLightAmbient * g_vMtrlAmbient);

	float4		vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	float4		vLook = vWorldPos - g_vCamPosition;
	float4		vReflect = reflect(normalize(g_vLightDir), vNormal);	

	float		fSpecular = pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 30.f);

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * fSpecular;

	return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
	PS_OUT_LIGHT			Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	float4		vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.0f;

	float4		vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	vector		vLightDir = vWorldPos - g_vLightPos;
	float		fDistance = length(vLightDir);

	float		fAtt = saturate((g_fLightRange - fDistance) / g_fLightRange);

	Out.vShade = g_vLightDiffuse * saturate(max(dot(normalize(vLightDir) * -1.f, vNormal), 0.f) + g_vLightAmbient * g_vMtrlAmbient);
	Out.vShade *= fAtt;
	
	float4		vLook = vWorldPos - g_vCamPosition;
	float4		vReflect = reflect(normalize(vLightDir), vNormal);

	float		fSpecular = pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 30.f);

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * fSpecular;
	Out.vSpecular *= fAtt;

	return Out;
}

/* 최종적ㅇ으로 480000 수행되는 쉐이더. */
PS_OUT PS_MAIN_FINAL(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector		vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (0.0f == vDiffuse.a)
		discard;
	vector		vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
	vector		vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    vector		SSAODesc = g_SSAOTexture.Sample(LinearSampler, In.vTexcoord);
	
    Out.vColor = vDiffuse * vShade + (vSpecular * 0.5f);
	Out.vColor *= SSAODesc;

	/* 현재 픽셀의 월드상의 위치를 구한다. */

	/* ProjPos.w == View.Z */
	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * 1000.0f;

	float4		vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	/* 라이트 뷰, 투영을 곱한다. */
	vector		vPosition = mul(vWorldPos, g_LightViewMatrix);
	vPosition = mul(vPosition, g_LightProjMatrix);

	float2		vTexcoord;

	vTexcoord.x = (vPosition.x / vPosition.w) * 0.5f + 0.5f;
	vTexcoord.y = (vPosition.y / vPosition.w) * -0.5f + 0.5f;

	vector		vLightDepthDesc = g_LightDepthTexture.Sample(LinearSampler, vTexcoord);

	/* vPosition.w : 현재 내가 그릴려고 했던 픽셀의 광원기준의 깊이. */
	/* vLightDepthDesc.x * 2000.f : 현재 픽셀을 광원기준으로  그릴려고 했던 위치에 이미 그려져있떤 광원 기준의 깊이.  */
	//if(vPosition.w - 0.1f > (vLightDepthDesc.x * 2000.f))
	//	Out.vColor *= 0.2f;		

	return Out;
}

PS_OUT PS_SSAO(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    vector DepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    vector NormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    float4 vNormal = float4(NormalDesc.xyz * 2.f - 1.f, 0.f);
    
    const float fFar = 1000.f;
    const int iNumSampleVertical = 7;
    const int iNumSampleHorizon = 7;
    
    float fViewZ = DepthDesc.y * fFar;
            
    vector vSrcWorldPos;
            
            /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
    vSrcWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vSrcWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vSrcWorldPos.z = DepthDesc.x;
    vSrcWorldPos.w = 1.f;

	        /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vSrcWorldPos *= fViewZ;

	        /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vSrcWorldPos = mul(vSrcWorldPos, g_ProjMatrixInv);

	        /* 로컬위치 * 월드행렬 */
    vSrcWorldPos = mul(vSrcWorldPos, g_ViewMatrixInv);
    
    float fWidth, fHeight;
    g_DepthTexture.GetDimensions(fWidth, fHeight);
    
    float fInvAmbient = 0.f;
    
    for (int i = iNumSampleHorizon / 2 * -1; i < iNumSampleHorizon / 2; ++i)
    {
        if (i == 0)
            continue;
        
        for (int j = iNumSampleVertical / 2 * -1; j < iNumSampleVertical / 2; ++j)
        {
            if (j == 0)
                continue;
            
            float2 vTexcoord = float2(In.vTexcoord.x + (1.f / fWidth * i), In.vTexcoord.y + (1.f / fHeight * j));
            
            DepthDesc = g_DepthTexture.Sample(PointSamplerClamp, vTexcoord);
            
            float fViewZ = DepthDesc.y * fFar;
            
            vector vDstWorldPos;
            
            /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
            vDstWorldPos.x = vTexcoord.x * 2.f - 1.f;
            vDstWorldPos.y = vTexcoord.y * -2.f + 1.f;
            vDstWorldPos.z = DepthDesc.x;
            vDstWorldPos.w = 1.f;

	        /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
            vDstWorldPos *= fViewZ;

	        /* 로컬위치 * 월드행렬 * 뷰행렬 */
            vDstWorldPos = mul(vDstWorldPos, g_ProjMatrixInv);

	        /* 로컬위치 * 월드행렬 */
            vDstWorldPos = mul(vDstWorldPos, g_ViewMatrixInv);
            vector vDirection = vDstWorldPos - vSrcWorldPos;
            
            fInvAmbient += max(0.0, dot(normalize(vNormal.xyz), normalize(vDirection.xyz))) * (1.0f / (1.0f + length(vDirection)));
        }
    }
    
    fInvAmbient = fInvAmbient / ((iNumSampleHorizon - 1) * (iNumSampleVertical - 1));
    if (fInvAmbient > 1.f)
        fInvAmbient = 1.f;
            
    Out.vColor = (1.f - fInvAmbient);
    
    return Out;
}



technique11 DefaultTechnique
{
	pass Debug
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass Light_Directional
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
	}

	pass Light_Point
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN_POINT();
	}

	pass Final
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN_FINAL();
	}

    pass SSAO
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SSAO();
    }
}