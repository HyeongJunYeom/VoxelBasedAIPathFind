#include "Gui_Object.h"
#include "GameInstance.h"

CGui_Object::CGui_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CGui_Object::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	GUI_OBJECT_DESC*		pDesc = { static_cast<GUI_OBJECT_DESC*>(pArg) };
	m_pFocus = pDesc->pFocus;
	m_pHovered = pDesc->pHovered;

	return S_OK;
}

void CGui_Object::Tick(_float fTimeDelta)
{
}

void CGui_Object::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
