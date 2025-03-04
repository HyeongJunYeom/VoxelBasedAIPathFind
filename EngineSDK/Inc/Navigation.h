#pragma once

#include "Component.h"
#include "PathIncludes.h"

BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		class CTransform*		pTransform = { nullptr };
	}NAVIGATION_DESC;

private:
	const static _float			ms_fMaxCostDetour;

private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& rhs);
	virtual ~CNavigation() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;
	void Priority_Tick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	void Late_Tick(_float fTimeDelta);

private:		// For.Move
	HRESULT Move_Sequence(_float fTimeDelta);

	HRESULT Move_Path(_float fTimeDelta);
	HRESULT Flocking(_float fTimeDelta);
	HRESULT Fall(_float fTimeDelta);
	void Recover_Wall(_fvector vStartPos, _fvector vDirection, const _float fRadius, _float3& vRecoveredDirectionFloat3);

public:
	_bool Is_Empty_Command() { return m_CommandInfoQueue.empty(); }

private:
	HRESULT Add_Supports();

public:
	void Set_MakeNewPass();
	HRESULT Get_StartIndex(_uint& iIndex);
	HRESULT Get_GoalIndex(_uint& iIndex);

	HRESULT Get_LastGoalIndex(_uint& iIndex);
	HRESULT Get_LastStartIndex(_uint& iIndex);

	HRESULT Get_CurPos_Index(_uint& iIndex);

	list<_float3> Get_BestPath_Positions_Float();
	list<_float3> Get_BestPath_Positions_Float_Optimized();
	const list<_uint>& Get_BestPath_Indices();
	const list<_uint>& Get_BestPath_Indices_Optimized();
	const vector<_float3>& Get_BestPath_Positions_Smooth();

	const unordered_set<_uint>& Get_OpenList();
	const unordered_set<_uint>& Get_CloseList();

	ALGORITHM Get_Algorithm();
	MAIN_HEURISTIC Get_MainHeuristic();
	void Set_Algorithm(ALGORITHM eAlgorithm);
	void Set_MainHeuristic(MAIN_HEURISTIC eHeuristic);

	TEST_MODE Get_OptimizerTestMode();
	void Set_OptimizerTestMode(TEST_MODE eTestMode);

	_float Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType);
	void Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight);

	_float Compute_Total_Dist_BestPath();
	_float Compute_Total_Dist_OptimizedPath();

	_llong Get_Make_Path_Time();
	_llong Get_Optimize_Path_Time();
	_llong Get_Smooth_Path_Time();

public:
	class CFormation* Get_Formation() { return m_pFormation; }

public:
	HRESULT Get_Cur_FlowFieldIndexTag(_uint& iIndexTag);
	HRESULT Clear_CommandQueue();
	void Add_CommandInfo(COMMAND_INFO& CommandInfo);

private:
	HRESULT Compute_Distance_From_Goal(const _uint iGoalIndex, _fvector vCurPos, _float& fDistance);

public:		/* For.Agent */
	_bool Is_Patrol() const;
	_bool Is_Move() const;
	_float Get_Speed() const;
	_float Get_Radius() const;
	_float Get_Height() const;
	_float Get_MaxWaitTime() const;
	_float Get_ObstacleRecognizeDist() const;

	void Set_Patrol(const _bool isPatrol);
	void Start_Move();
	void Stop_Move();
	void Next_Move();
	void Set_Speed(const _float fSpeed);
	void Set_Radius(const _float fRadius);
	void Set_Height(const _float fHeight);
	void Set_MaxWaitTime(const _float fMaxWaitTime);
	void Set_ObstacleRecognizeDist(const _float fObstacleRecognizeDist);

public:		/* For.Formation */
	HRESULT Add_FormationGroup(const _uint iGroupIndexTag);
	HRESULT Erase_FormationGroup(const _uint iGroupIndexTag);

	HRESULT SetUp_Formation_Path();
	HRESULT SetUp_Formation_PathTest(const _uint iFormationIndexTag);

private:

private:
	HRESULT Ready_Detour_Static_Obstacle(const list<_uint>& OriginPath, list<_uint>& NewPath, const _uint iMoveTargetIndex, _uint& iNewStartIndex, vector<_uint>& NewGoals, const vector<_uint>& MyReserveIndices);
	HRESULT Ready_Detour_Dynamic_Obstacle(const list<_uint>& OriginPath, list<_uint>& NewPath, const _uint iMoveTargetIndex, _uint& iNewStartIndex, vector<_uint>& NewGoals, const vector<_uint>& MyReserveIndices);
	HRESULT End_Detour();


private:	/* Supports */
	class CAgent*							m_pAgent = { nullptr };
	class CBoid*							m_pBoid = { nullptr };
	class CFormation*						m_pFormation = { nullptr };

private:
	class CTransform*						m_pTransformCom = { nullptr };

	_bool									m_isMove = { false };
	_bool									m_isArrived = { false };
	_bool									m_isFall = { false };
	_float									m_fAccFallSpeed = { 0.f };

private:
	vector<COMMAND_INFO*>					m_CommandInfoQueue;

public:
	static void Set_ReserveSensortive(const _float fSensortive);
	static _float Get_ReserveSensortive();

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END