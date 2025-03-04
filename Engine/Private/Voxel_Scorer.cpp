#include "..\Public\Voxel_Scorer.h"
#include "GameInstance.h"
CVoxel_Scorer::CVoxel_Scorer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Scorer::Initialize(void* pArg)
{
	return S_OK;
}

_float CVoxel_Scorer::Copute_VoxelOpennessScore(_uint iVoxelIndex)
{
	Clear_Data();
	Update_Data();
	Compute_Openness_Score_BFS(iVoxelIndex);

	//	m_fTotalScore = static_cast<_float>(m_iNumReachedNeighbor) / powf(m_iMaxStep * 2.f, 3.f) * 10.f;

	m_fTotalScore /= powf(m_iMaxStep * 2.f, 2.f);
	m_fTotalScore *= 10.f;

	return m_fTotalScore;
}

void CVoxel_Scorer::Clear_Data()
{
	m_VisitedIndices.clear();
	m_fTotalScore = 0.f;
	m_iNumReachedNeighbor = 0;
	m_fCurVoxelWorldSize = 0.f;
}

void CVoxel_Scorer::Update_Data()
{
	m_fCurVoxelWorldSize = m_pGameInstance->Get_WorldSize_Voxel();
}

void CVoxel_Scorer::Compute_Openness_Score_BFS(const _uint iCurIndex, const _uint iCurStep)
{
	if (m_iMaxStep <= iCurStep)
		return;


	vector<_uint>			NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC) };
	const _float			fCurNeighborScore = { m_fStepScore * static_cast<_float>(iCurStep + 1) };

	vector<_uint>			NewVisitedNeighbors;
	
	for (auto& iNeighborIndex : NeighborIndices)
	{
		/*auto iter{ m_VisitedIndices.find(iNeighborIndex) };
		if (iter != m_VisitedIndices.end())
			continue;*/

		VOXEL_ID			eID = { VOXEL_ID::_END };
		if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eID, VOXEL_LAYER::_STATIC)))
		{
			MSG_BOX(TEXT("Failed To Get_VoxelID => Compute_OpennessScoreBFS"));
			continue;
		}
		
		if (eID == VOXEL_ID::_FLOOR)
		{
			++m_iNumReachedNeighbor;
			NewVisitedNeighbors.push_back(iNeighborIndex);


			_float			fDist;
			if (FAILED(Compute_Dist(iCurIndex, iNeighborIndex, fDist)))
			{
				MSG_BOX(TEXT("Failed To Compute_Dist => Compute_OpennessScoreBFS"));
				continue;
			}

			//	m_VisitedIndices.emplace(iNeighborIndex);
			m_fTotalScore += 1.f / fDist;
		}
	}

	for (auto& iNewNeighborIndex : NewVisitedNeighbors)
	{
		Compute_Openness_Score_BFS(iNewNeighborIndex, iCurStep + 1);
	}
}

HRESULT CVoxel_Scorer::Compute_Dist(const _uint iSrcIndex, const _uint iDstIndex, _float& fDist)
{
	_uint3			vSrcIndexPos, vDstIndexPos;
	if (FAILED(m_pGameInstance->Get_IndexPosition_Voxel(iSrcIndex, vSrcIndexPos)) ||
		FAILED(m_pGameInstance->Get_IndexPosition_Voxel(iDstIndex, vDstIndexPos)))
		return E_FAIL;

	_vector			vSrcLocalPos = { XMLoadUInt3(&vSrcIndexPos) }; 
	_vector			vDstLocalPos = { XMLoadUInt3(&vDstIndexPos) };	

	_float			fLocalDist = { XMVectorGetX(XMVector3Length(vDstLocalPos - vSrcLocalPos)) };

	//	fDist = fLocalDist * m_fCurVoxelWorldSize;
	fDist = fLocalDist;

	return S_OK;
}

CVoxel_Scorer* CVoxel_Scorer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Scorer*		pInstance = { new CVoxel_Scorer{pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel_Scorer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Scorer::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
