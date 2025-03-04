#include "stdafx.h"
#include "..\Public\Camera_Free.h"

CCamera_Free::CCamera_Free(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CCamera{ pDevice, pContext }
{

}

CCamera_Free::CCamera_Free(const CCamera_Free & rhs)
	: CCamera{ rhs }
{

}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Free::Initialize(void * pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	
	CAMERA_FREE_DESC*	pCameraFree = (CAMERA_FREE_DESC*)pArg;

	m_fMouseSensor = pCameraFree->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Tick(_float fTimeDelta)
{
}

void CCamera_Free::Tick(_float fTimeDelta)
{
	

	if (m_pGameInstance->Get_KeyState('A') == PRESSING)
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}
	if (m_pGameInstance->Get_KeyState('D') == PRESSING)
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}
	if (m_pGameInstance->Get_KeyState('W') == PRESSING)
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (m_pGameInstance->Get_KeyState('S') == PRESSING)
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	

	if (m_pGameInstance->Get_KeyState(VK_MBUTTON) == PRESSING)
	{
		POINT		ptDeltaPos = { m_pGameInstance->Get_MouseDeltaPos() };
		_vector		vUp = { XMVectorSet(0.f, 1.f, 0.f, 0.f) };

		if ((_long)0 != ptDeltaPos.x)
		{
			m_pTransformCom->Turn(vUp, fTimeDelta * (_float)ptDeltaPos.x * m_fMouseSensor);
		}

		_vector		vRight = { m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) };

		if ((_long)0 != ptDeltaPos.y)
		{
			m_pTransformCom->Turn(vRight, fTimeDelta * (_float)ptDeltaPos.y * m_fMouseSensor);
		}
	}

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0.f, 0.f, 0.f, 1.f));

	//m_fFovy = XMConvertToRadians(120.f);

	__super::Bind_PipeLines();
}

void CCamera_Free::Late_Tick(_float fTimeDelta)
{
}

HRESULT CCamera_Free::Render()
{
	return S_OK;
}

CCamera_Free * CCamera_Free::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CCamera_Free*		pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CCamera_Free::Clone(void * pArg)
{
	CCamera_Free*		pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	__super::Free();

}
