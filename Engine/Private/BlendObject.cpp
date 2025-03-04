#include "..\Public\BlendObject.h"

#include "GameInstance.h"

CBlendObject::CBlendObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	
{
	
}

CBlendObject::CBlendObject(const CBlendObject & rhs)
	: CGameObject{ rhs }
{
	
}


HRESULT CBlendObject::Initialize_Prototype()
{
	return S_OK;
}

/* 실제 게임내엣 사용되는 객체가 호출하는 함수다. */
HRESULT CBlendObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	
	return S_OK;
}

void CBlendObject::Tick(_float fTimeDelta)
{
}

void CBlendObject::Late_Tick(_float fTimeDelta)
{

}

HRESULT CBlendObject::Render()
{
	return S_OK;
}



void CBlendObject::Compute_CamDistance()
{
	_vector		vDir = m_pGameInstance->Get_CamPosition_Vector() - m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	m_fCamDistance = XMVectorGetX(XMVector3Length(vDir));
}

void CBlendObject::Free()
{
	__super::Free();



	
}
