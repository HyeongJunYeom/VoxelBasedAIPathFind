#include "..\Public\Obstacle_Tool.h"
#include "Obstacle.h"

CObstacle_Tool::CObstacle_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGui_Object{ pDevice, pContext }
{
}

HRESULT CObstacle_Tool::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	OBSTACLE_TOOL_DESC* pDesc = { static_cast<OBSTACLE_TOOL_DESC*>(pArg) };
	m_pObstacles = pDesc->pObstacles;
	m_pCurObjectIndex = pDesc->pCurObjectIndex;

	if (nullptr == m_pObstacles ||
		nullptr == m_pCurObjectIndex)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CObstacle_Tool::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	ImGui::Begin("Obstacle Tool");

	if (ImGui::IsWindowFocused())
		*m_pFocus = true;
	if (ImGui::IsWindowHovered())
		*m_pHovered = true;

	_int		iCurObjectIndex = { *m_pCurObjectIndex };
	if (0 <= iCurObjectIndex &&
		static_cast<_int>(m_pObstacles->size()) > iCurObjectIndex)
	{
		for (auto pComponent : (*m_pObstacles))
		{
			if (nullptr == pComponent)
				continue;

			CObstacle*		pObstacle = { static_cast<CObstacle*>(pComponent) };
			pObstacle->Tick(fTimeDelta);
		}
	}

	ImGui::End();
}

CObstacle_Tool* CObstacle_Tool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CObstacle_Tool*			pInstance = { new CObstacle_Tool(pDevice, pContext) };
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CObstacle_Tool"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObstacle_Tool::Free()
{
	__super::Free();
}
