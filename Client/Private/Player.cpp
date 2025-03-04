#include "stdafx.h"
#include "..\Public\Player.h"


#include "Body_Player.h"
#include "Weapon.h"

CPlayer::CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer & rhs)
	: CGameObject{ rhs }
{

}

HRESULT CPlayer::Initialize_Prototype()
{
	

	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_PartObjects()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(3.f, 3.f, 3.f);

	return S_OK;
}

void CPlayer::Priority_Tick(_float fTimeDelta)
{
}

void CPlayer::Tick(_float fTimeDelta)
{
	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(fTimeDelta);

	if (PRESSING == m_pGameInstance->Get_KeyState(VK_UP))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}

	if (PRESSING == m_pGameInstance->Get_KeyState(VK_DOWN))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (PRESSING == m_pGameInstance->Get_KeyState(VK_LEFT))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, -1.f, 0.f, 0.f), fTimeDelta);
	}

	if (PRESSING == m_pGameInstance->Get_KeyState(VK_RIGHT))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
	}

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
	m_pObstacle->Tick(fTimeDelta);	
}

void CPlayer::Late_Tick(_float fTimeDelta)
{
	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

#ifdef _DEBUG
	m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif
}

HRESULT CPlayer::Render()
{

	return S_OK;
}

HRESULT CPlayer::Add_Components()
{
	/* Com_Collider */
	CBounding_AABB::BOUNDING_AABB_DESC		ColliderDesc{};

	/* 로컬상의 정보를 셋팅한다. */
	ColliderDesc.vSize = _float3(0.8f, 1.2f, 0.8f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	CObstacle::OBSTACLE_DESC			ObstacleDesc;
	ObstacleDesc.pTransform = m_pTransformCom;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Obstacle_Cube"),
		TEXT("Com_Obstacle"), (CComponent**)&m_pObstacle, &ObstacleDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Add_PartObjects()
{


	/* For.Part_Body */
	CPartObject*		pBodyObject = { nullptr };
	CBody_Player::BODY_DESC	BodyDesc{};

	BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	BodyDesc.pState = &m_eState;

	pBodyObject = dynamic_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Body_Player"), &BodyDesc));
	if (nullptr == pBodyObject)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Body"), pBodyObject);
	
	return S_OK;
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pObstacle);

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);

	m_PartObjects.clear();
}
