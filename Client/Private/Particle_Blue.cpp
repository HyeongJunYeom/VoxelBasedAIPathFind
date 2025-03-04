#include "stdafx.h"
#include "..\Public\Particle_Blue.h"

#include "GameInstance.h"


CParticle_Blue::CParticle_Blue(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CPartObject{ pDevice, pContext }
{
}

CParticle_Blue::CParticle_Blue(const CParticle_Blue & rhs)
	: CPartObject{ rhs }
{
}

HRESULT CParticle_Blue::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParticle_Blue::Initialize(void * pArg)
{	
	PARTOBJECT_DESC*	pPartObjectDesc = (PARTOBJECT_DESC*)pArg;

	pPartObjectDesc->fSpeedPerSec = 10.f;
	pPartObjectDesc->fRotationPerSec = XMConvertToRadians(90.0f);	

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	return S_OK;
}

void CParticle_Blue::Priority_Tick(_float fTimeDelta)
{
}

void CParticle_Blue::Tick(_float fTimeDelta)
{
	m_pVIBufferCom->Spread(fTimeDelta);

	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));

}

void CParticle_Blue::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CParticle_Blue::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CParticle_Blue::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxInstance_Rect"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Snow"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Instance_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CParticle_Blue::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	return S_OK;
}

CParticle_Blue * CParticle_Blue::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CParticle_Blue*		pInstance = new CParticle_Blue(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CParticle_Blue"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CParticle_Blue::Clone(void * pArg)
{
	CParticle_Blue*		pInstance = new CParticle_Blue(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CParticle_Blue"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CParticle_Blue::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
}
