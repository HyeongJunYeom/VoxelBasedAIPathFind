#include "Obstacle_Manager.h"
#include "Obstacle.h"
#include "GameInstance.h"

CObstacle_Manager::CObstacle_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance {CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CObstacle_Manager::Initialize()
{
	return S_OK;
}

void CObstacle_Manager::Priority_Tick(_float fTimeDelta)
{
}

void CObstacle_Manager::Tick(_float fTimeDelta)
{
}

void CObstacle_Manager::Late_Tick(_float fTimeDelta)
{
}

HRESULT CObstacle_Manager::Add_Obstacle(CObstacle* pObstacle)
{


	return S_OK;
}

HRESULT CObstacle_Manager::Erase_Obstacle(CObstacle* pObstacle)
{


	return S_OK;
}

CObstacle_Manager* CObstacle_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CObstacle_Manager*		pInstance = { new CObstacle_Manager(pDevice, pContext) };

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CObstacle_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObstacle_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (auto pObstacle : m_Static_Obstacles)
	{
		Safe_Release(pObstacle);
		pObstacle = nullptr;
	}
	m_Static_Obstacles.clear();

	for (auto pObstacle : m_Dynamic_Obstacles)
	{
		Safe_Release(pObstacle);
		pObstacle = nullptr;
	}
	m_Static_Obstacles.clear();
}
