#include "PathFinder_Voxel.h"
#include "VIBuffer_Line.h"
#include "Path_Optimizer.h"
#include "Path_Rounder.h"
#include "GameInstance.h"

_float CPathFinder_Voxel::ms_fWeightY = { 1.f };

CPathFinder_Voxel::CPathFinder_Voxel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPathFinder_Voxel::Initialize(void* pArg)
{
	if (FAILED(Add_Supports()))
		return E_FAIL;

	m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_DISTANCE)] = 1.f;
	m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_OPENNESS)] = 1.f;

	return S_OK;
}
/*
//HRESULT CPathFinder_Voxel::Make_Path()
//{
//	//	TODO: 단순 벽피하기가아닌 이동불가지형을 나중에 한번에 처리하게 하기
//	VOXEL_ID eStartID, eGoalID;
//	if (FAILED(m_pGameInstance->Get_VoxelID(*m_pStartIndex, eStartID, VOXEL_LAYER::_STATIC)) ||
//		FAILED(m_pGameInstance->Get_VoxelID(*m_pGoalIndex, eGoalID, VOXEL_LAYER::_STATIC)) ||
//		VOXEL_ID::_WALL == eStartID ||
//		VOXEL_ID::_WALL == eGoalID)
//		return E_FAIL;
//
//	if (FAILED(Clear_PathInfos()))
//		return E_FAIL;
//
//	{
//		auto start = std::chrono::high_resolution_clock::now();
//
//		if (FAILED(Make_Route()))
//		{
//			cout << "Engine : Failed Make Route()" << endl;
//			return E_FAIL;
//		}
//		auto end = std::chrono::high_resolution_clock::now();
//		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//		m_MakePathTime = duration.count();
//	}
//
//	//if (true == m_isActiveOpimize)
//	//{
//	//	auto start = std::chrono::high_resolution_clock::now();
//
//	//	m_pPath_Optimizer->Compute_Optimized_Path(*m_pBestPathIndices);
//
//	//	auto end = std::chrono::high_resolution_clock::now();
//	//	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//	//	m_OptimizedPathTime = duration.count();
//	//}
//
//	//if (true == m_isActiveOpimize)
//	//{
//	//	auto start = std::chrono::high_resolution_clock::now();
//
//	//	//if(true == m_isActiveOpimize)
//	//	//	m_pPath_Rounder->Compute_Smooth_Path(Get_BestPath_Positions_Float_Optimized());
//	//	//else
//	//	//	m_pPath_Rounder->Compute_Smooth_Path(Get_BestPath_Positions_Float());
//
//	//	if (true == m_isActiveOpimize)
//	//		m_pPath_Rounder->Compute_Smooth_Path_Bazier(Get_BestPath_Positions_Float_Optimized());
//	//	else
//	//		m_pPath_Rounder->Compute_Smooth_Path_Bazier(Get_BestPath_Positions_Float());
//
//
//	//	auto end = std::chrono::high_resolution_clock::now();
//	//	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//	//	m_SmoothPathTime = duration.count();
//	//}
//
//	return S_OK;
//}*/

HRESULT CPathFinder_Voxel::Make_Path(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices)
{
	_float			fMaxCost = { m_fMaxCost };
	m_fMaxCost = FLT_MAX;
	//	목표점이 없으면 탐색 실패
	if (true == GoalIndices.empty())
		return E_FAIL;
	 
	//	시작점, 목표점들 중 경로가 되지 않는 복셀들이 포함된 경우 경로 탐색 실패
	VOXEL_ID		eStartID, eGoalID;
	if (FAILED(m_pGameInstance->Get_VoxelID(iStartIndex, eStartID, VOXEL_LAYER::_STATIC)) ||
		VOXEL_ID::_WALL == eStartID)
		return E_FAIL;

	for (const auto iGoalIndex : GoalIndices)
	{
		if (FAILED(m_pGameInstance->Get_VoxelID(iGoalIndex, eGoalID, VOXEL_LAYER::_STATIC)) ||
			VOXEL_ID::_WALL == eGoalID)
			return E_FAIL;
	}
	
	//	기존 경로 정보 클리어
	if (FAILED(Clear_PathInfos()))
		return E_FAIL;

	{
		auto start = std::chrono::high_resolution_clock::now();

		if (FAILED(Make_Route(PathIndices, NodeInfos, iStartIndex, GoalIndices, isDetourStaticObstacle, isDetourDynamicObstacle, MyReserveIndices, fMaxCost)))
		{
			cout << "Engine : Failed Make Route()" << endl;
			return E_FAIL;
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_MakePathTime = static_cast<_llong>(duration.count());
	}

	/*
	if (true == m_isActiveOpimize)
	{
		auto start = std::chrono::high_resolution_clock::now();

		m_pPath_Optimizer->Compute_Optimized_Path(*m_pBestPathIndices);

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_OptimizedPathTime = duration.count();
	}

	if (true == m_isActiveOpimize)
	{
		auto start = std::chrono::high_resolution_clock::now();

		//if(true == m_isActiveOpimize)
		//	m_pPath_Rounder->Compute_Smooth_Path(Get_BestPath_Positions_Float_Optimized());
		//else
		//	m_pPath_Rounder->Compute_Smooth_Path(Get_BestPath_Positions_Float());

		if (true == m_isActiveOpimize)
			m_pPath_Rounder->Compute_Smooth_Path_Bazier(Get_BestPath_Positions_Float_Optimized());
		else
			m_pPath_Rounder->Compute_Smooth_Path_Bazier(Get_BestPath_Positions_Float());


		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_SmoothPathTime = duration.count();
	}
	*/

	return S_OK;
}

HRESULT CPathFinder_Voxel::Make_Path_Inverse(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, _uint iGoalIndex, const _bool isDetourStaticObstacle)
{
	_float			fMaxCost = { m_fMaxCost };
	m_fMaxCost = FLT_MAX;
	//	목표점이 없으면 탐색 실패
	if (true == StartIndices.empty())
		return E_FAIL;

	//	시작점, 목표점들 중 경로가 되지 않는 복셀들이 포함된 경우 경로 탐색 실패
	VOXEL_ID		eStartID, eGoalID;
	if (FAILED(m_pGameInstance->Get_VoxelID(iGoalIndex, eGoalID, VOXEL_LAYER::_STATIC)) ||
		VOXEL_ID::_WALL == eGoalID)
		return E_FAIL;

	for (const auto iStartIndex : StartIndices)
	{
		if (FAILED(m_pGameInstance->Get_VoxelID(iStartIndex, eStartID, VOXEL_LAYER::_STATIC)) ||
			VOXEL_ID::_WALL == eStartID)
			return E_FAIL;
	}

	//	기존 경로 정보 클리어
	if (FAILED(Clear_PathInfos()))
		return E_FAIL;

	{
		auto start = std::chrono::high_resolution_clock::now();

		if (FAILED(Make_Route_Inverse(NodeInfos, StartIndices, iGoalIndex, isDetourStaticObstacle, fMaxCost)))
		{
			cout << "Engine : Failed Make Route()" << endl;
			return E_FAIL;
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_MakePathTime = static_cast<_llong>(duration.count());
	}

	/*
	if (true == m_isActiveOpimize)
	{
		auto start = std::chrono::high_resolution_clock::now();

		m_pPath_Optimizer->Compute_Optimized_Path(*m_pBestPathIndices);

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_OptimizedPathTime = duration.count();
	}

	if (true == m_isActiveOpimize)
	{
		auto start = std::chrono::high_resolution_clock::now();

		//if(true == m_isActiveOpimize)
		//	m_pPath_Rounder->Compute_Smooth_Path(Get_BestPath_Positions_Float_Optimized());
		//else
		//	m_pPath_Rounder->Compute_Smooth_Path(Get_BestPath_Positions_Float());

		if (true == m_isActiveOpimize)
			m_pPath_Rounder->Compute_Smooth_Path_Bazier(Get_BestPath_Positions_Float_Optimized());
		else
			m_pPath_Rounder->Compute_Smooth_Path_Bazier(Get_BestPath_Positions_Float());


		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_SmoothPathTime = duration.count();
	}
	*/

	return S_OK;
}

list<_float3> CPathFinder_Voxel::Get_BestPath_Positions_Float_Optimized()
{
	return list<_float3>();
}

const list<_uint>& CPathFinder_Voxel::Get_BestPath_Indices_Optimized()
{
	return m_pPath_Optimizer->Get_OptimizedBestPath();
}

const vector<_float3>& CPathFinder_Voxel::Get_BestPath_Positions_Smooth()
{
	return m_pPath_Rounder->Get_SmoothPath();
}

HRESULT CPathFinder_Voxel::Clear_PathInfos()
{
	m_CloseList.clear();
	m_OpenList.clear();

	return S_OK;
}

const unordered_set<_uint>& CPathFinder_Voxel::Get_OpenList()
{
	return m_OpenList;
}

const unordered_set<_uint>& CPathFinder_Voxel::Get_CloseList()
{
	return m_CloseList;
}

_bool CPathFinder_Voxel::IsInBound(_uint iIndex)
{
	if (0 > iIndex ||
		g_iVoxelIndexLimit <= iIndex)
		return false;

	return true;
}

void CPathFinder_Voxel::Set_Algorithm(ALGORITHM eAlgorithm)
{
	if (ALGORITHM::_END <= eAlgorithm)
		return;

	m_eAlgorithm = eAlgorithm;
}

void CPathFinder_Voxel::Set_MainHeuristic(MAIN_HEURISTIC eHeuristic)
{
	if (MAIN_HEURISTIC::_END <= eHeuristic)
		return;

	m_eHeuristic = eHeuristic;
}

TEST_MODE CPathFinder_Voxel::Get_OptimizerTestMode()
{
	return m_pPath_Optimizer->Get_TestMode();
}

void CPathFinder_Voxel::Set_OptimizerTestMode(TEST_MODE eTestMode)
{
	m_pPath_Optimizer->Set_TestMode(eTestMode);
}

_float CPathFinder_Voxel::Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType)
{
	if (HEURISTIC_TYPE::_END <= eHeuriticType)
		return 0.f;

	return m_SubHeuristicWeights[static_cast<_uint>(eHeuriticType)];
}

void CPathFinder_Voxel::Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight)
{
	if (HEURISTIC_TYPE::_END <= eHeuriticType)
		return;

	m_SubHeuristicWeights[static_cast<_uint>(eHeuriticType)] = fmaxf(fWeight, -1.f);
}

_float CPathFinder_Voxel::Compute_G_Cost(_fvector vCurPos, const vector<_float3>& GoalPoses)
{
	return _float();
}

_float CPathFinder_Voxel::Compute_H_Cost(_fvector vCurPos, const vector<_float3>& GoalPoses, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices)
{
	if (true == isDetourStaticObstacle)
	{
		_float3				vCurPosFloat3;
		XMStoreFloat3(&vCurPosFloat3, vCurPos);

		_uint3				vIndexPos;
		if (FAILED(m_pGameInstance->Get_IndexPosition_Voxel(vCurPosFloat3, vIndexPos)))
			return FLT_MAX;

		if (true == m_pGameInstance->Is_Exist_Voxel(vIndexPos, VOXEL_LAYER::_OBSTACLE_STATIC))
			return FLT_MAX;
	}

	if (true == isDetourDynamicObstacle)
	{
		_float3				vCurPosFloat3;
		XMStoreFloat3(&vCurPosFloat3, vCurPos);


		_uint3				vIndexPos;
		_uint				iVoxelIndex;
		if (FAILED(m_pGameInstance->Get_IndexPosition_Voxel(vCurPosFloat3, vIndexPos)) ||
			FAILED(m_pGameInstance->Get_Index_Voxel(vIndexPos, iVoxelIndex)))
			return FLT_MAX;

		if(MyReserveIndices.end() == find(MyReserveIndices.begin(), MyReserveIndices.end(), iVoxelIndex) &&
			true == m_pGameInstance->Is_Exist_Voxel(vIndexPos, VOXEL_LAYER::_OBSTACLE_DYNAMIC))
			return FLT_MAX;
	}

	_vector			vNearGoalPos, vTempPos;
	_vector			vLastGoalPos = { XMLoadFloat3(&GoalPoses.back()) };

	_float			fNearDist = { FLT_MAX };

	for (const auto& vGoalPosFloat3 : GoalPoses)
	{
		vTempPos = XMLoadFloat3(&vGoalPosFloat3);
		_float		fNewDist = { XMVectorGetX(XMVector3Length(vTempPos - vCurPos)) };
		if (fNearDist > fNewDist)
		{
			fNearDist = fNewDist;
			vNearGoalPos = vTempPos;
		}
	}

	_vector			vResultPos = { (vNearGoalPos + vLastGoalPos) * 0.5f };

	_float			fHeuristicCost{};
	if (0.f <= m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_DISTANCE)])
	{
		if (MAIN_HEURISTIC::_EUCLIDEAN == m_eHeuristic)
		{
			fHeuristicCost += XMVectorGetX(XMVector3Length(vResultPos - vCurPos))
				* m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_DISTANCE)];
		}

		else if (MAIN_HEURISTIC::_MANHATTAN == m_eHeuristic)
		{
			_vector			vDirection = { vResultPos - vCurPos };
			fHeuristicCost += (fabsf(XMVectorGetX(vDirection))
					+ fabsf(XMVectorGetY(vDirection))
					+ fabsf(XMVectorGetZ(vDirection)))
				* m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_DISTANCE)];
		}
	}

	if (0.f <= m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_HEIGHT)])
	{
		_float			fHeightDiff = { fabsf(XMVectorGetY(vResultPos) - XMVectorGetY(vCurPos)) };

		fHeuristicCost += fHeightDiff 
			* m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_HEIGHT)];
	}

	if (0.f <= m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_OPENNESS)])
	{
		_float3			vCurPosFloat3;
		XMStoreFloat3(&vCurPosFloat3, vCurPos);

		_byte			bScore;
		m_pGameInstance->Get_VoxelOpenessScore(vCurPosFloat3, bScore, VOXEL_LAYER::_STATIC);
		fHeuristicCost += ((100.f - static_cast<_float>(bScore)) / 100.f) * m_SubHeuristicWeights[static_cast<_uint>(HEURISTIC_TYPE::_OPENNESS)] * 100.f;
	}	

	return fHeuristicCost;
}

_vector CPathFinder_Voxel::Apply_Height_Weight(_fvector vSrc)
{
	return XMVectorSetY(vSrc, XMVectorGetY(vSrc) * ms_fWeightY);
}

_float CPathFinder_Voxel::Compute_TotalNeighborScore(_uint iIndex)
{
	vector<_uint>			NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iIndex, VOXEL_LAYER::_STATIC) };
	_uint					iNumNeighbor{};

	for (auto& iNeighborIndex : NeighborIndices)
	{
		VOXEL_ID			eID = { VOXEL_ID::_END };
		if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eID, VOXEL_LAYER::_STATIC)))
		{
			MSG_BOX(TEXT("Failed GetVoxel ID => Compute_TotalNeighborScore"));
			continue;
		}

		if (VOXEL_ID::_FLOOR == eID)
			++iNumNeighbor;
	}

	return static_cast<_float>(iNumNeighbor) * 1.f;
}

_float CPathFinder_Voxel::Compute_Total_Dist_BestPath(const list<_uint>& PathIndices)
{
	_float3			vWorldPos = {};
	_vector			vLastPos, vCurPos;

	_float			fTotalDist = {};

	_bool			isSetLast = {};

	for (auto& iIndex : PathIndices)
	{
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iIndex, vWorldPos)))
		{
			cout << "Failed To Compute Path Length Best Path" << endl;
			return 0.f;
		}

		vCurPos = XMLoadFloat3(&vWorldPos);

		if (false == isSetLast)
			isSetLast = true;
		else
			fTotalDist += XMVectorGetX(XMVector3Length(vLastPos - vCurPos));

		vLastPos = vCurPos;
	}


	return fTotalDist;
}

_float CPathFinder_Voxel::Compute_Total_Dist_OptimizedPath()
{
	_float3			vWorldPos = {};
	_vector			vLastPos, vCurPos;

	_float			fTotalDist = {};

	_bool			isSetLast = {};


	for (auto& iIndex : m_pPath_Optimizer->Get_OptimizedBestPath())
	{
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iIndex, vWorldPos)))
		{
			cout << "Failed To Compute Path Length Best Path" << endl;
			return 0.f;
		}

		vCurPos = XMLoadFloat3(&vWorldPos);

		if (false == isSetLast)
			isSetLast = true;
		else
			fTotalDist += XMVectorGetX(XMVector3Length(vLastPos - vCurPos));

		vLastPos = vCurPos;
	}


	return fTotalDist;
}

HRESULT CPathFinder_Voxel::Add_Supports()
{
	m_pPath_Optimizer = CPath_Optimizer::Create(m_pDevice, m_pContext);
	if (nullptr == m_pPath_Optimizer)
		return E_FAIL;

	m_pPath_Rounder = CPath_Rounder::Create(m_pDevice, m_pContext);
	if (nullptr == m_pPath_Rounder)
		return E_FAIL;

	return S_OK;
}

struct CMP {
	_bool operator() (const pair<_float, _uint> Src, const pair<_float, _uint> Dst) {
		return Src.first > Dst.first;
	}
};

typedef priority_queue<pair<_float, _uint>, vector<pair<_float, _uint>>, CMP> AStarPQ;

HRESULT CPathFinder_Voxel::Make_Route(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices, const _float fMaxCost)
{
	_float3			vStartPosFloat3 = { ConvertToWorldPosition(iStartIndex) };
	_vector			vStartPos = { XMVectorSetW(XMLoadFloat3(&vStartPosFloat3), 1.f) };

	vector<_float3>			GoalPoses;
	for (const auto iGoalIndex : GoalIndices)
		GoalPoses.emplace_back(ConvertToWorldPosition(iGoalIndex));

	AStarPQ			PriorityQ;
	PriorityQ.push({ 0.f, iStartIndex });
	m_OpenList.emplace(iStartIndex);
	NodeInfos[iStartIndex].fG_Cost = 0.f;
	NodeInfos[iStartIndex].iParentIndex = iStartIndex;

	while (false == PriorityQ.empty())
	{
		_uint			iCurIndex = { PriorityQ.top().second };
		PriorityQ.pop();

		const _bool			isGoal = { GoalIndices.end() != find(GoalIndices.begin(), GoalIndices.end(), iCurIndex) };
		if (true == isGoal)
		{
			Make_BestList(PathIndices, NodeInfos, iStartIndex, iCurIndex);
			return S_OK;
		}

		m_CloseList.emplace(iCurIndex);
		Pop_OpenList(iCurIndex);

		vector<_uint>	NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC) };

		//	TODO: 플래그 등으로 워커블 처리 isWalkable
		if (false == m_isClimbWall)
		{
			auto		iter{ NeighborIndices.begin() };
			while (iter != NeighborIndices.end())
			{
				VOXEL_ID			eID;
				if (FAILED(m_pGameInstance->Get_VoxelID(*iter, eID, VOXEL_LAYER::_STATIC)))
				{
					cout << "HRESULT CPathFinder_Voxel::Make_Route_GBFS()" << endl;
					cout << "if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eID)))" << endl;
				}

				//	TODO: 플래그처리
				if (eID == VOXEL_ID::_WALL)
					iter = NeighborIndices.erase(iter);

				else
					++iter;
			}
		}

		_float3			vCurPosFloat3 = { ConvertToWorldPosition(iCurIndex) };
		_vector			vCurPos = { XMVectorSetW(XMLoadFloat3(&vCurPosFloat3), 1.f) };
		_float			fCur_G_Cost = { NodeInfos[iCurIndex].fG_Cost };

		for (auto& iNeighborIndex : NeighborIndices)
		{
			if (true == Is_Close(iNeighborIndex))
				continue;

			_float3			vNeighborPosFloat3 = { ConvertToWorldPosition(iNeighborIndex) };
			_vector			vNeighborPos = { XMVectorSetW(XMLoadFloat3(&vNeighborPosFloat3), 1.f) };

			_float			fDistToNeighbor = { XMVectorGetX(XMVector3Length(vNeighborPos - vCurPos)) };

			_float			fNeighbor_G_Cost = { fCur_G_Cost + fDistToNeighbor };
			if(fNeighbor_G_Cost > fMaxCost)
				continue;

			_float			fNeighbor_H_Cost = { Compute_H_Cost(vNeighborPos, GoalPoses, isDetourStaticObstacle, isDetourDynamicObstacle, MyReserveIndices) };
			if (fNeighbor_H_Cost >= FLT_MAX)
				continue;

			_float			fNeighbor_Total_Cost = {};
			if (m_eAlgorithm == ALGORITHM::_A_STAR)
				fNeighbor_Total_Cost = fNeighbor_G_Cost + fNeighbor_H_Cost;
			else if(ALGORITHM::_GBFS == m_eAlgorithm)
				fNeighbor_Total_Cost = fNeighbor_H_Cost;

			if (true == Is_Open(iNeighborIndex) &&
				fNeighbor_G_Cost >= NodeInfos[iNeighborIndex].fG_Cost)
				continue;

			NodeInfos[iNeighborIndex].iParentIndex = iCurIndex;
			NodeInfos[iNeighborIndex].fG_Cost = fNeighbor_G_Cost;
			m_OpenList.emplace(iNeighborIndex);
			PriorityQ.push({ fNeighbor_Total_Cost, iNeighborIndex });
		}
	}

	//	cout << "Engine : Cant make path" << endl;

	return E_FAIL;
}

HRESULT CPathFinder_Voxel::Make_Route_Inverse(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle, const _float fMaxCost)
{
	_float3			vGoalPosFloat3 = { ConvertToWorldPosition(iGoalIndex) };
	_vector			vGoalPos = { XMVectorSetW(XMLoadFloat3(&vGoalPosFloat3), 1.f) };

	vector<_float3>			StartPoses;
	unordered_set<_uint>	StartIndicesSet;
	for (const auto iStartIndex : StartIndices)
	{
		StartPoses.emplace_back(ConvertToWorldPosition(iStartIndex));
		StartIndicesSet.insert(iStartIndex);
	}


	AStarPQ			PriorityQ;
	PriorityQ.push({ 0.f, iGoalIndex });
	m_OpenList.emplace(iGoalIndex);
	NodeInfos[iGoalIndex].fG_Cost = 0.f;
	NodeInfos[iGoalIndex].iParentIndex = iGoalIndex;

	while (false == PriorityQ.empty())
	{
		_uint			iCurIndex = { PriorityQ.top().second };
		PriorityQ.pop();

		const _bool			isGoal = { StartIndices.end() != find(StartIndices.begin(), StartIndices.end(), iCurIndex) };
		if (true == isGoal)
		{
			auto iter = StartIndicesSet.find(iCurIndex);
			if (iter != StartIndicesSet.end())
				StartIndicesSet.erase(iter);
		}

		if (true == StartIndicesSet.empty())
		{
			return S_OK;
		}

		m_CloseList.emplace(iCurIndex);
		Pop_OpenList(iCurIndex);

		vector<_uint>	NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC) };

		//	TODO: 플래그 등으로 워커블 처리 isWalkable
		if (false == m_isClimbWall)
		{
			auto		iter{ NeighborIndices.begin() };
			while (iter != NeighborIndices.end())
			{
				VOXEL_ID			eID;
				if (FAILED(m_pGameInstance->Get_VoxelID(*iter, eID, VOXEL_LAYER::_STATIC)))
				{
					cout << "HRESULT CPathFinder_Voxel::Make_Route_GBFS()" << endl;
					cout << "if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eID)))" << endl;
				}

				//	TODO: 플래그처리
				if (eID == VOXEL_ID::_WALL)
					iter = NeighborIndices.erase(iter);

				else
					++iter;
			}
		}

		_float3			vCurPosFloat3 = { ConvertToWorldPosition(iCurIndex) };
		_vector			vCurPos = { XMVectorSetW(XMLoadFloat3(&vCurPosFloat3), 1.f) };
		_float			fCur_G_Cost = { NodeInfos[iCurIndex].fG_Cost };

		for (auto& iNeighborIndex : NeighborIndices)
		{
			if (true == Is_Close(iNeighborIndex))
				continue;

			_float3			vNeighborPosFloat3 = { ConvertToWorldPosition(iNeighborIndex) };
			_vector			vNeighborPos = { XMVectorSetW(XMLoadFloat3(&vNeighborPosFloat3), 1.f) };

			_float			fDistToNeighbor = { XMVectorGetX(XMVector3Length(vNeighborPos - vCurPos)) };

			_float			fNeighbor_G_Cost = { fCur_G_Cost + fDistToNeighbor };
			if (fNeighbor_G_Cost > fMaxCost)
				continue;

			_float			fNeighbor_H_Cost = { Compute_H_Cost(vNeighborPos, StartPoses, isDetourStaticObstacle, false, vector<_uint>()) };
			if (fNeighbor_H_Cost >= FLT_MAX)
				continue;

			_float			fNeighbor_Total_Cost = {};
			if (m_eAlgorithm == ALGORITHM::_A_STAR)
				fNeighbor_Total_Cost = fNeighbor_G_Cost + fNeighbor_H_Cost;
			else if (ALGORITHM::_GBFS == m_eAlgorithm)
				fNeighbor_Total_Cost = fNeighbor_H_Cost;

			if (true == Is_Open(iNeighborIndex) &&
				fNeighbor_G_Cost >= NodeInfos[iNeighborIndex].fG_Cost)
				continue;

			NodeInfos[iNeighborIndex].iParentIndex = iCurIndex;
			NodeInfos[iNeighborIndex].fG_Cost = fNeighbor_G_Cost;
			m_OpenList.emplace(iNeighborIndex);
			PriorityQ.push({ fNeighbor_Total_Cost, iNeighborIndex });
		}
	}

	//	cout << "Engine : Cant make path" << endl;

	return E_FAIL;
}

void CPathFinder_Voxel::Make_BestList(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const _uint iGoalIndex)
{
	PathIndices.clear();

	_uint		iCurIndex = { iGoalIndex };

	while (iCurIndex != iStartIndex)
	{
		PathIndices.push_front(iCurIndex);
		iCurIndex = NodeInfos[iCurIndex].iParentIndex;
	}

	PathIndices.push_front(iStartIndex);
}

_float3 CPathFinder_Voxel::ConvertToWorldPosition(_uint iVoxelIndex)
{
	_float3			vCurPosFloat3{};
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndex, vCurPosFloat3)))
	{
#ifdef _DEBUG
		cout << "HRESULT CPathFinder_Voxel::Make_Route_To_Position(_uint iStartIdx, _uint iGoalIdx)" << endl;
		cout << "if (FAILED(m_pGameInstance->Get_Position_Voxel(iNeighborIndex, vNeighborPosFloat3)))" << endl;
#endif
	}

	return vCurPosFloat3;
}

_bool CPathFinder_Voxel::Is_Open(_uint iIndex)
{
	return m_OpenList.end() != m_OpenList.find(iIndex);
}

_bool CPathFinder_Voxel::Is_Close(_uint iIndex)
{
	return m_CloseList.end() != m_CloseList.find(iIndex);
}

_bool CPathFinder_Voxel::Pop_OpenList(_uint iIndex)
{
	return Pop_ListBase(m_OpenList, iIndex);

}

_bool CPathFinder_Voxel::Pop_CloseList(_uint iIndex)
{
	return Pop_ListBase(m_CloseList, iIndex);
}

_bool CPathFinder_Voxel::Pop_ListBase(unordered_set<_uint>& List, _uint iIndex)
{
	auto iter{ List.find(iIndex) };

	if (List.end() == iter)
		return false;

	List.erase(iter);
	return true;
}

CPathFinder_Voxel* CPathFinder_Voxel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CPathFinder_Voxel* pInstance = { new CPathFinder_Voxel{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPathFinder_Voxel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPathFinder_Voxel::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Release(m_pPath_Optimizer);
	Safe_Release(m_pPath_Rounder);
}
