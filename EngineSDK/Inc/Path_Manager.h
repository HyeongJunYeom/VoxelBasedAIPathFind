#pragma once

#include "Base.h"

BEGIN(Engine)

class CPath_Manager final : public CBase
{
private:
	CPath_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPath_Manager() = default;

public:
	HRESULT Initialize();
	void Priority_Tick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	HRESULT Render();

public:
	HRESULT Add_Render_Path(class CNavigation* pNavigation);
	HRESULT Add_Render_Path(const list<_uint>& PathIndices);
	HRESULT Set_FlowFieldRenderIndex(const _uint iIndexTag);

private:
	HRESULT Add_Components();
	HRESULT Add_PathFinder();	

public:
	const unordered_map<_uint, _float3>& Get_FlowField(const _uint iIndexTag);

private:
	void Render_Path(const list<_float3>& Path, _float4 vColor);
	void Render_Round_Path(const vector<_float3>& Path, _float4 vColor);

	void Render_Directions(const vector < pair<_float3, _float3>>& Directions, const _float4& vColor);

public:	/* PathFinder_Voxel*/
	HRESULT Make_Path_Voxel(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices);
	HRESULT Make_Path_Inverse_Voxel(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle);

	list<_float3> Get_BestPath_Positions_Float_Optimized();
	const list<_uint>& Get_BestPath_Indices_Optimized();
	const vector<_float3>& Get_BestPath_Positions_Smooth();
	HRESULT Clear_PathInfos();

	const unordered_set<_uint>& Get_OpenList();
	const unordered_set<_uint>& Get_CloseList();

	ALGORITHM Get_Algorithm();
	MAIN_HEURISTIC Get_MainHeuristic();
	void Set_Algorithm(ALGORITHM eAlgorithm);
	void Set_MainHeuristic(MAIN_HEURISTIC eHeuristic);
	void Set_MaxCost(const _float fMaxCost);
	TEST_MODE Get_OptimizerTestMode();
	void Set_OptimizerTestMode(TEST_MODE eTestMode);
	_float Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType);
	void Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight);

	_float Compute_Total_Dist_BestPath(const list<_uint>& PathIndices);
	_float Compute_Total_Dist_OptimizedPath();

	_llong Get_Make_Path_Time();
	_llong Get_Optimize_Path_Time();
	_llong Get_Smooth_Path_Time();

public:	/* PathFinder_FlowFiled */
	HRESULT Make_VectorFiled_Daijkstra(const vector<_uint>& GoalIndices, const _uint iNumStartAgent, _uint& iIndexTag);
	HRESULT Make_VectorFiled_A_Star(const _uint iGoalIndex, const vector<_uint>& StartIndices, const _uint iNumStartAgent, _uint& iIndexTag);
	HRESULT Get_Direction_FlowField(const _float3& vCurPos, const _uint iIndexTag, _float3& vDirection);
	HRESULT Get_ParentIndex_FlowField(const _float3& vCurPos, const _uint iIndexTag, _uint& iParentIndex);

	_float Get_Distance_From_Goal_FlowField(const _uint iVoxelIndex, const _uint iIndexTag);

	HRESULT AddRef_FlowField(const _uint iIndexTag);
	HRESULT Release_FlowField(const _uint iIndexTag);

	vector<_uint> Get_Active_FlowField_RefCnt();
	vector<_uint> Get_Active_FlowField_Indices();

	HRESULT Convert_BestList_From_FlowField(list<_uint>& BestList, const _uint iIndexTag, const _uint iStartIndex);

private:
	vector<_float3> Make_SmoothPath(_fvector vStart, _fvector vEnd, _fvector vDir, _float fRadius, _uint iNumSample);

private:
	ID3D11Device*							m_pDevice = { nullptr };
	ID3D11DeviceContext*					m_pContext = { nullptr };
	class CGameInstance*					m_pGameInstance = { nullptr };

private:
	class CVIBuffer_Line*					m_pVIBuffer_Line = { nullptr };
	class CShader*							m_pShader = { nullptr };

	class CVIBuffer_Instance_Line*			m_pVIBuffer_Instance_Line = { nullptr };
	class CShader*							m_pShader_Instance_Line = { nullptr };

private:
	class CPathFinder_FlowField*			m_pPathFinder_FlowField = { nullptr };
	class CPathFinder_Voxel*				m_pPathFinder_Voxel = { nullptr };

	vector<pair<_float3, _float3>>			m_Instance_Poses_Buffer;

private:
	vector<class CNavigation*>				m_RenderObjects;
	vector<list<_float3>>					m_RenderPathes;
	_uint									m_iRenderFlowFieldIndex = {};

public:
	static CPath_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END