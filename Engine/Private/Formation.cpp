#include "Formation.h"
#include "GameInstance.h"

CFormation::CFormation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CFormation::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	FORMATION_DESC*			pDesc = { static_cast<FORMATION_DESC*>(pArg) };
	m_pTransform = pDesc->pTransform;
	m_pSpeed = pDesc->pSpeed;
	m_pMinSpeed = pDesc->pMinSpeed;
	m_pMaxSpeed = pDesc->pMaxSpeed;
	m_pMaxWaitTime = pDesc->pMaxWaitTime;

	if (nullptr == m_pTransform ||
		nullptr == m_pSpeed ||
		nullptr == m_pMinSpeed ||
		nullptr == m_pMaxSpeed ||
		nullptr == m_pMaxWaitTime)
		return E_FAIL;

	Safe_AddRef(m_pTransform);

	return S_OK;
}

void CFormation::Tick(_float fTimeDelta)
{
}

HRESULT CFormation::Add_FormationGroup(const _uint iGroupIndex)
{
	return m_pGameInstance->Add_Formation(this, iGroupIndex);
}

HRESULT CFormation::Erase_FormationGroup(const _uint iGroupIndex)
{
	return m_pGameInstance->Erase_Formation(this, iGroupIndex);
}

HRESULT CFormation::Update_CurrentRatio()
{
	if (nullptr == m_pTransform)
		return E_FAIL;

	_vector			vCurAgentPos = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
	_float3			vCurAgentPosFloat3;
	XMStoreFloat3(&vCurAgentPosFloat3, vCurAgentPos);

	_uint			iCurAgentVoxelIndex;
	if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurAgentPosFloat3, iCurAgentVoxelIndex)))
		return E_FAIL;

	auto		iter = m_PathRatios.find(iCurAgentVoxelIndex);
	if (iter != m_PathRatios.end())
		m_fLastRatio = iter->second;

	return S_OK;
}

HRESULT CFormation::Update_Speed(const _float fMainRatio, const _float fMinRatio, const _float fMaxRatio)
{
	const _float			fAgnetRatioInv = { 1.f - m_fLastRatio };
	const _float			fMinSpeed = { Get_MinSpeed() };
	const _float			fMaxSpeed = { Get_MaxSpeed() };
	const _float			fRatioRange = { fMaxRatio - fMinRatio };

	_float			fClampedRatio = { m_fLastRatio };
	fClampedRatio = fmaxf(fClampedRatio, fMinRatio);
	fClampedRatio = fminf(fClampedRatio, fMaxRatio);

	const _float			fResultRatio = { (fClampedRatio - fMinRatio) / fRatioRange };
	const _float			fResultSpeed = { (fMinSpeed * (fResultRatio)) + (fMaxSpeed * (1.f - fResultRatio)) };

	if (fResultRatio < 0.5f)
	{
		Set_MaxWaitTime(0.f);
	}
	else
	{
		Set_MaxWaitTime(0.25f);
	}

	Set_Speed(fResultSpeed);

	return S_OK;
}

CFormation* CFormation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CFormation*			pInstance = { new CFormation(pDevice, pContext) };
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CFormation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFormation::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pTransform);
}
