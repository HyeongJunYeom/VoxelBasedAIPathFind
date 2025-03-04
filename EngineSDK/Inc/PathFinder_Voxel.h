#pragma once

#include "Base.h"
#include "PathIncludes.h"

BEGIN(Engine)

class CPathFinder_Voxel final : public CBase
{	
private:
	CPathFinder_Voxel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPathFinder_Voxel(const CPathFinder_Voxel& rhs) = delete;
	virtual ~CPathFinder_Voxel() = default;

public:
	HRESULT Initialize(void* pArg);

public:
	HRESULT Make_Path(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices);
	HRESULT Make_Path_Inverse(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, _uint iGoalIndex, const _bool isDetourStaticObstacle);

public:
	list<_float3> Get_BestPath_Positions_Float_Optimized();
	const list<_uint>& Get_BestPath_Indices_Optimized();
	const vector<_float3>& Get_BestPath_Positions_Smooth();
	HRESULT Clear_PathInfos();

public:	/* For.Debug */
	const unordered_set<_uint>& Get_OpenList();
	const unordered_set<_uint>& Get_CloseList();

private:
	_bool IsInBound(_uint iIndex);

public:
	ALGORITHM Get_Algorithm() { return m_eAlgorithm; }
	MAIN_HEURISTIC Get_MainHeuristic() { return m_eHeuristic; }
	void Set_Algorithm(ALGORITHM eAlgorithm);
	void Set_MainHeuristic(MAIN_HEURISTIC eHeuristic);
	void Set_MaxCost(const _float fMaxCost) { m_fMaxCost = fMaxCost; }

public:	/* For.Optimizer */
	TEST_MODE Get_OptimizerTestMode();
	void Set_OptimizerTestMode(TEST_MODE eTestMode);

public:	/* For.Heuristic_Weights */
	_float Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType);
	void Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight);

private:
	_float Compute_G_Cost(_fvector vCurPos, const vector<_float3>& GoalPoses);
	_float Compute_H_Cost(_fvector vCurPos, const vector<_float3>& GoalPoses, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices);
	_vector Apply_Height_Weight(_fvector vSrc);

private:
	_float Compute_TotalNeighborScore(_uint iIndex);

public:
	_float Compute_Total_Dist_BestPath(const list<_uint>& PathIndices);
	_float Compute_Total_Dist_OptimizedPath();

private:
	HRESULT Add_Supports();

private:
	HRESULT Make_Route(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices, const _float fMaxCost);
	HRESULT Make_Route_Inverse(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle, const _float fMaxCost);

	//	HRESULT Make_Route_GBFS(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices);
	//	HRESULT Make_Route(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices);
	//	HRESULT Make_Route_GBFS_Inverse(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle);
	//	HRESULT Make_Route_A_Star_Inverse(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle);
	void Make_BestList(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, const _uint iStartIndex, const _uint iGoalIndex);

	_float3 ConvertToWorldPosition(_uint iVoxelIndex);

	_bool Is_Open(_uint iIndex);
	_bool Is_Close(_uint iIndex);

	_bool Pop_OpenList(_uint iIndex);
	_bool Pop_CloseList(_uint iIndex);
	_bool Pop_ListBase(unordered_set<_uint>&List, _uint iIndex);

public:
	_llong Get_Make_Path_Time() { return m_MakePathTime; }
	_llong Get_Optimize_Path_Time() { return m_OptimizedPathTime; }
	_llong Get_Smooth_Path_Time() { return m_SmoothPathTime; }	

private:
	class CGameInstance*					m_pGameInstance = { nullptr };
	ID3D11Device*							m_pDevice = { nullptr };
	ID3D11DeviceContext*					m_pContext = { nullptr };

private:
	list<_uint>								m_BestPathIndices;
	unordered_set<_uint>					m_OpenList;
	unordered_set<_uint>					m_CloseList;

	_float									m_fMaxCost = { FLT_MAX };
	_float									m_fTotalDist = {};

	ALGORITHM								m_eAlgorithm = { ALGORITHM::_A_STAR };
	MAIN_HEURISTIC							m_eHeuristic = { MAIN_HEURISTIC::_EUCLIDEAN };

	HEURISTIC_WEIGHTS						m_SubHeuristicWeights = {};

	//	TODO:		이동능력 플래그형태로 지형별로 배칭하여 관리하기 
	_bool									m_isClimbWall = { false };			

	class CPath_Optimizer*					m_pPath_Optimizer = { nullptr };
	class CPath_Rounder*					m_pPath_Rounder = { nullptr };

	static _float							ms_fWeightY;

	_llong									m_MakePathTime = {};
	_llong									m_OptimizedPathTime = {};
	_llong									m_SmoothPathTime = {};

	_bool									m_isActiveOpimize = { true };
	_bool									m_isActiveSmooth = { true };

public:
	static CPathFinder_Voxel* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, void* pArg);
	virtual void Free() override;
};

END