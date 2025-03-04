#include "Voxel_Link.h"
#include "GameInstance.h"

CVoxel_Link::CVoxel_Link(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Link::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CVoxel_Link::Render()
{
	return S_OK;
}

CVoxel_Link* CVoxel_Link::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Link*		pInstance = { new CVoxel_Link{pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel_Link"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Link::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
