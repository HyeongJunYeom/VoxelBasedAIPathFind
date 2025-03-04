#include "..\Public\Path_Detour.h"
#include "GameInstance.h"

CPath_Detour::CPath_Detour(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPath_Detour::Initialize(void* pArg)
{
	return S_OK;
}

CPath_Detour* CPath_Detour::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPath_Detour*			pInstance = { new CPath_Detour{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize(nullptr)))
	{
		MSG_BOX(TEXT("Failed To Created : CPath_Detour"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPath_Detour::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
