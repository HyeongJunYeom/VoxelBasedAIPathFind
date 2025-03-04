#include "Boid_Manager.h"
#include "GameInstance.h"

#include "Boid.h"

CBoid_Manager::CBoid_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CBoid_Manager::Initialize()
{
	return S_OK;
}

HRESULT CBoid_Manager::Add_Boid(CBoid* pBoid)
{
	if (nullptr == pBoid)
		return E_FAIL;

	auto iter{ find(m_Boids.begin(), m_Boids.end(), pBoid) };
	if (iter != m_Boids.end())
		return E_FAIL;

	m_Boids.push_back(pBoid);
	Safe_AddRef(pBoid);

	return S_OK;
}

HRESULT CBoid_Manager::Erase_Boid(CBoid* pBoid)
{
	if (nullptr == pBoid)
		return E_FAIL;

	auto iter{ find(m_Boids.begin(), m_Boids.end(), pBoid) };
	if (iter == m_Boids.end())
		return E_FAIL;

	Safe_Release(*iter);
	m_Boids.erase(iter);

	return S_OK;
}

HRESULT CBoid_Manager::Get_Neighbor_Position_Boid(const _uint iNeighborIndex, _float3& vNeighborPos)
{
	if (m_Boids.size() <= iNeighborIndex)
		return E_FAIL;

	_vector			vNeighborPosVector = { m_Boids[iNeighborIndex]->Get_Transform()->Get_State_Vector(CTransform::STATE_POSITION) };
	XMStoreFloat3(&vNeighborPos, vNeighborPosVector);

	return S_OK;
}

HRESULT CBoid_Manager::Get_Neighbor_Indices_Boid(CBoid* pBoid, const _float fNeighborRange, vector<_uint>& NeighborIndices)
{
	_uint		iBoidIndex;
	if (FAILED(Find_BoidIndex(pBoid, iBoidIndex)))
		return E_FAIL;

	_vector		vPosition = { m_Boids[iBoidIndex]->Get_Transform()->Get_State_Vector(CTransform::STATE_POSITION) };

	const _uint			iNumBoid = { static_cast<_uint>(m_Boids.size()) };
	for (_uint iDstBoidIndex = 0; iDstBoidIndex < iNumBoid; ++iDstBoidIndex)
	{
		CBoid*			pBoid = m_Boids[iDstBoidIndex];

		_vector			vBoidPos = { pBoid->Get_Transform()->Get_State_Vector(CTransform::STATE_POSITION) };
		_float			fDist = { XMVectorGetX(XMVector3Length(vPosition - vBoidPos)) };

		if (fDist <= fNeighborRange)
		{
			NeighborIndices.push_back(iDstBoidIndex);
		}
	}

	return S_OK;
}

HRESULT CBoid_Manager::Get_Neighbor_FlowFieldIndex_Boid(const _uint iNeighborIndex, _uint& iFlowFieldIndexTag)
{
	if (m_Boids.size() <= iNeighborIndex)
		return E_FAIL;

	iFlowFieldIndexTag = m_Boids[iNeighborIndex]->Get_CurFlowFieldIndex();
	return S_OK;
}

HRESULT CBoid_Manager::Find_BoidIndex(CBoid* pBoid, _uint& iBoidIndex)
{
	auto iter{ find(m_Boids.begin(), m_Boids.end(), pBoid) };
	if (iter == m_Boids.end())
		return E_FAIL;

	iBoidIndex = static_cast<_uint>(iter - m_Boids.begin());
	return S_OK;
}

CBoid_Manager* CBoid_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBoid_Manager*		pInstance = { new CBoid_Manager(pDevice, pContext) };
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CBoid_Manager"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBoid_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (auto pBoid : m_Boids)
	{
		Safe_Release(pBoid);
		pBoid = nullptr;
	}
	m_Boids.clear();
}
