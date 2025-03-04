#include "stdafx.h"
#include "..\Public\ForkLift.h"

CForkLift::CForkLift(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CForkLift::CForkLift(const CForkLift & rhs)
	: CGameObject{ rhs }
{

}

HRESULT CForkLift::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CForkLift::Initialize(void * pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;	

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(5.0f, 2.f, 5.0f, 1.f));

	return S_OK;
}

void CForkLift::Priority_Tick(_float fTimeDelta)
{
}

void CForkLift::Tick(_float fTimeDelta)
{
}

void CForkLift::Late_Tick(_float fTimeDelta)
{


	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_FIELD, this);
}

HRESULT CForkLift::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;



	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}


	return S_OK;
}

HRESULT CForkLift::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxModel"), 
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CForkLift::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;	

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CForkLift * CForkLift::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CForkLift*		pInstance = new CForkLift(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CForkLift"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CForkLift::Clone(void * pArg)
{
	CForkLift*		pInstance = new CForkLift(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CForkLift"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CForkLift::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);	
	Safe_Release(m_pModelCom);
}
