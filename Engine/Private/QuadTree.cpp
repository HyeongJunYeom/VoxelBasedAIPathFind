#include "..\Public\QuadTree.h"
#include "GameInstance.h"

CQuadTree::CQuadTree()
{

}

HRESULT CQuadTree::Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{	
	m_iCorners[CORNER_LT] = iLT;
	m_iCorners[CORNER_RT] = iRT;
	m_iCorners[CORNER_RB] = iRB;
	m_iCorners[CORNER_LB] = iLB;

	if (1 == m_iCorners[CORNER_RT] - m_iCorners[CORNER_LT])
		return S_OK;

	m_iCenter = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RB]) >> 1;

	_uint iLC, iTC, iRC, iBC;

	iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
	iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
	iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
	iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

	m_pChildren[CORNER_LT] = CQuadTree::Create(m_iCorners[CORNER_LT], iTC, m_iCenter, iLC);
	m_pChildren[CORNER_RT] = CQuadTree::Create(iTC, m_iCorners[CORNER_RT], iRC, m_iCenter);
	m_pChildren[CORNER_RB] = CQuadTree::Create(m_iCenter, iRC, m_iCorners[CORNER_RB], iBC);
	m_pChildren[CORNER_LB] = CQuadTree::Create(iLC, m_iCenter, iBC, m_iCorners[CORNER_LB]);

	return S_OK;
}

HRESULT CQuadTree::SetUp_Neighbors()
{
	if (nullptr == m_pChildren[CORNER_LT]->m_pChildren[CORNER_LT])
		return S_OK;

	m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChildren[CORNER_RT];
	m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChildren[CORNER_LB];

	m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChildren[CORNER_LT];
	m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChildren[CORNER_RB];

	m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChildren[CORNER_LB];
	m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChildren[CORNER_RT];

	m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChildren[CORNER_RB];
	m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChildren[CORNER_LT];

	if (nullptr != m_pNeighbors[NEIGHBOR_RIGHT])
	{
		m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChildren[CORNER_LT];
		m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChildren[CORNER_LB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_BOTTOM])
	{
		m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChildren[CORNER_LT];
		m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChildren[CORNER_RT];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_LEFT])
	{
		m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChildren[CORNER_RT];
		m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChildren[CORNER_RB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_TOP])
	{
		m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChildren[CORNER_LB];
		m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChildren[CORNER_RB];
	}

	for (size_t i = 0; i < CORNER_END; i++)
	{
		m_pChildren[i]->SetUp_Neighbors();
	}

	return S_OK;
}

void CQuadTree::Culling(CGameInstance* pGameInstance, const _float3* pVerticesPos, _uint* pIndices, _uint* pNumIndices )
{
	/* �� �̻� �п��� ���� �ʴ´ٸ� �ڽĵ鿡 ���� �񱳸� �� �ʿ䰡 ����. */
	if (nullptr == m_pChildren[CORNER_LT] || 
		true == isDraw(pGameInstance, pVerticesPos))
	{
		_bool	isIn[4] = {
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]])),
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RT]])),
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RB]])),
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LB]])),
		};

		_bool	isDraw[NEIGHBOR_END] = { true, true,true,true };

		if(nullptr != m_pNeighbors[NEIGHBOR_LEFT])
			isDraw[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->isDraw(pGameInstance, pVerticesPos);
		if (nullptr != m_pNeighbors[NEIGHBOR_TOP])
			isDraw[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->isDraw(pGameInstance, pVerticesPos);
		if (nullptr != m_pNeighbors[NEIGHBOR_RIGHT])
			isDraw[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->isDraw(pGameInstance, pVerticesPos);
		if (nullptr != m_pNeighbors[NEIGHBOR_BOTTOM])
			isDraw[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->isDraw(pGameInstance, pVerticesPos);



		if (true == isDraw[NEIGHBOR_LEFT] &&
			true == isDraw[NEIGHBOR_TOP] &&
			true == isDraw[NEIGHBOR_RIGHT] &&
			true == isDraw[NEIGHBOR_BOTTOM])
		{
			/* �����Ѥ��� �� �ﰡ���� �׷����ٸ�. */
			if (true == isIn[0] ||
				true == isIn[1] ||
				true == isIn[2])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}

			/* ���� �Ʒ� �ﰡ���� �׷����ٸ�. */
			if (true == isIn[0] ||
				true == isIn[2] ||
				true == isIn[3])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}

			return;
		}

		_uint iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
		_uint iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
		_uint iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
		_uint iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

		if (true == isIn[0] ||
			true == isIn[2] ||
			true == isIn[3])
		{
			if (false == isDraw[NEIGHBOR_LEFT])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iLC;

				pIndices[(*pNumIndices)++] = iLC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}

			if (false == isDraw[NEIGHBOR_BOTTOM])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iBC;

				pIndices[(*pNumIndices)++] = iBC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
		}

		if (true == isIn[0] ||
			true == isIn[1] ||
			true == isIn[2])
		{
			if (false == isDraw[NEIGHBOR_TOP])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}

			if (false == isDraw[NEIGHBOR_RIGHT])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}
		}

		return;
	}

	/* ���� ��Ʈ���� ����ü�� �浹���Ͽ� �浹�Ǿ��ٸ�.?? : �ڽ�����Ʈ���� ���� �񱳸� �ݺ������� �����ϱ� ���ؼ�. */

	_float		fRange = XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]]) - XMLoadFloat3(&pVerticesPos[m_iCenter])).m128_f32[0];

	// if (���� ��Ʈ���� ����ü�� �浹���Ͽ� �浹�Ǿ��ٸ�.)
	if(pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCenter]), fRange))
	{
		for (size_t i = 0; i < CORNER_END; i++)
		{
			m_pChildren[i]->Culling(pGameInstance, pVerticesPos, pIndices, pNumIndices);
		}
	}
	else
	{
		int a = 10;
	}
}

_bool CQuadTree::isDraw(CGameInstance * pGameInstance, const _float3 * pVerticesPos)
{
	_vector		vCamPos = pGameInstance->Get_CamPosition_Vector();

	_float		fCenterDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCenter]) - vCamPos));

	_float		fWidth = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RT]]) - XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]])));

	if (fCenterDistance * 0.2f > fWidth)
		return true;

	return false;	
}

CQuadTree * CQuadTree::Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	CQuadTree*		pInstance = new CQuadTree();

	if (FAILED(pInstance->Initialize(iLT, iRT, iRB, iLB)))
	{
		MSG_BOX(TEXT("Failed to Created : CQuadTree"));
		Safe_Release(pInstance);
	}	

	return pInstance;
}

void CQuadTree::Free()
{
	__super::Free();

	for (_int i = 0; i < CORNER_END; i++) 
	{
		if (m_pChildren[i] != nullptr)
			Safe_Release(m_pChildren[i]);
	}

}

