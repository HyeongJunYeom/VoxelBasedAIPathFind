#include "Path_Optimizer.h"
#include "GameInstance.h"

CPath_Optimizer::CPath_Optimizer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPath_Optimizer::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CPath_Optimizer::Render()
{
	return S_OK;
}

void CPath_Optimizer::Compute_Optimized_Path(const list<_uint>& BestList)
{
	if (true == BestList.empty())
		return;

	m_BestPathIndices_Optimized.clear();

	auto			iter{ BestList.begin() };
	m_BestPathIndices_Optimized.push_back(*iter);


	if (TEST_MODE::_DEFAULT == m_eTestMode)
	{
		++iter;
		while (iter != BestList.end())
		{
			if (false == m_pGameInstance->Is_Reach_TargetIndex(m_BestPathIndices_Optimized.back(), *iter, VOXEL_LAYER::_STATIC))
			{
				--iter;

				if (*iter == m_BestPathIndices_Optimized.back())
					++iter;

				m_BestPathIndices_Optimized.push_back(*iter);
			}
			++iter;
		}

		if(BestList.back() != m_BestPathIndices_Optimized.back())
			m_BestPathIndices_Optimized.push_back(BestList.back());
	}

	else if (TEST_MODE::_INFITINTE == m_eTestMode)
	{
		++iter;
		while (iter != BestList.end())
		{
			_uint		iCurBest = { *iter };
			auto		iterCurStart{ iter };

			while (iter != BestList.end())
			{
				if (true == m_pGameInstance->Is_Reach_TargetIndex(m_BestPathIndices_Optimized.back(), *iter, VOXEL_LAYER::_STATIC))
				{
					iCurBest = *iter;
				}

				++iter;
			}

			if (0 != iCurBest)
				m_BestPathIndices_Optimized.push_back(iCurBest);
			else if (++iterCurStart != BestList.end())
				m_BestPathIndices_Optimized.push_back(*iterCurStart);

			iter = find(BestList.begin(), BestList.end(), m_BestPathIndices_Optimized.back());
			++iter;
		}
	}
}

const list<_uint>& CPath_Optimizer::Get_OptimizedBestPath()
{
	return m_BestPathIndices_Optimized;
}

CPath_Optimizer* CPath_Optimizer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CPath_Optimizer* pInstance = { new CPath_Optimizer{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPathFinder_Voxel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPath_Optimizer::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
