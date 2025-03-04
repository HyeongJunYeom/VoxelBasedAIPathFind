#pragma once

#include "Base.h"
#include "VoxelIncludes.h"
#include "PathIncludes.h"

BEGIN(Engine)

class CAgent final : public CBase
{
public:
	typedef struct tagAgentDesc
	{
		class CTransform*		pTransform = { nullptr };
		_bool*					pMove = { nullptr };
		_bool*					pArrived = { nullptr };
		vector<COMMAND_INFO*>*	pCommandInfoQueue = { nullptr };
	}AGENT_DESC;

private:
	CAgent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAgent(const CAgent& rhs) = delete;
	virtual ~CAgent() = default;

public:
	HRESULT Initialize(void* pArg);
	void Tick(_float fTimeDelta);

public:
	void Move_Seq(_float fTimeDelta);
	void Move_To_Target(_float& fRemainTime, _fvector vTargetPos);
	void Rotate_To_Target(_float& fRemainTime, _fvector vTargetPos);
	void Check_NextPathes_StaticObstacle();
	void Check_NextPathes_DynamicReserve();
	void Check_NextPathes_All_Blocked();

private:
	_bool Is_All_BlockToGoal();

private:
	HRESULT Sense_Reserve();

	HRESULT Clear_ReserveIndices();
	HRESULT Sense_CurPos();
	HRESULT Sense_NextPoses();
	HRESULT Reserve_SensoredIndices();
	void Reserve_NextPathes();
	void Update_CurReserveIndices();

	vector<_uint> Compute_NextVoxelIndices_InRange(const _float fDist);

public:
	_float* Get_Speed_Ptr() { return &m_fSpeed; }
	_float* Get_MinSpeed_Ptr() { return &m_fMinSpeed; }
	_float* Get_MaxSpeed_Ptr() { return &m_fMaxSpeed; }
	_float* Get_MaxWaitTime_Ptr() { return &m_fMaxWaitTime; }

public:
	_bool Is_Patrol() const { return m_isPatrol; }
	_bool Is_NeedMakeNewPath();
	_bool Is_NeedMakeNewPath_Detour_Static();
	_bool Is_NeedMakeNewPath_Detour_Dynamic();
	_bool Is_NeedMakeNewPath_All_Blocked_To_Goal();
	_float Get_Speed() const { return m_fSpeed; }
	_float Get_MinSpeed() const { return m_fMinSpeed; }
	_float Get_MaxSpeed() const { return m_fMaxSpeed; }
	_float Get_Radius() const { return m_fRadius; }
	_float Get_Height() const { return m_fHeight; }
	_float Get_MaxWaitTime() const { return m_fMaxWaitTime; }
	_uint Get_MoveIndex() { return m_iMoveIndex; }
	_float Get_ObstacleRecognizeDist() const {return m_fObstacleRecognizeDist; }
	const vector<_uint>& Get_ReserveIndices() { return m_ReserveIndices; }

	void Start_Move();
	void Start_Wait();
	void Stop_Move();
	void Set_MakeNewPass();
	void Set_Patrol(const _bool isPatrol) { m_isPatrol = isPatrol; }
	void Set_Speed(const _float fSpeed) { if (0.f > fSpeed) return; m_fSpeed = fSpeed; }
	void Set_MinSpeed(const _float fMinSpeed) { if (0.f > fMinSpeed) return; m_fMinSpeed = fMinSpeed; }
	void Set_MaxSpeed(const _float fMaxSpeed) { if (0.f > fMaxSpeed) return; m_fMaxSpeed = fMaxSpeed; }
	void Set_Radius(const _float fRadius) { if (0.f > fRadius) return; m_fRadius = fRadius; }
	void Set_Height(const _float fHeight) { if (0.f > fHeight) return; m_fHeight = fHeight; }
	void Set_MaxWaitTime(const _float fMaxWaitTime) { if (0.f > fMaxWaitTime) return; m_fMaxWaitTime = fMaxWaitTime; }
	void Set_ObstacleRecognizeDist(const _float fObstacleRecognizeDist) { if (0.f > fObstacleRecognizeDist) return; m_fObstacleRecognizeDist = fObstacleRecognizeDist; }

private:
	HRESULT Get_CurrentVoxelIndex(_uint& iCurVoxelIndex, const _uint iPathIndex);
	HRESULT Get_NextVoxelIndex(_uint& iNextVoxelIndex, _uint& iPathIndex);


private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*			m_pContext = { nullptr };

private:
	class CTransform*				m_pTransform = { nullptr };
	vector<COMMAND_INFO*>*			m_pCommandInfoQueue = { nullptr };


private:
	_float							m_fRadius = { 1.f };
	_float							m_fHeight = { 1.f };

	_float							m_fSpeed = {};
	_float							m_fMinSpeed = {};
	_float							m_fMaxSpeed = {};
	_float							m_fRotatePerSec = {};
	_float							m_fAccelation = {};

	_bool*							m_pMove = { nullptr };
	_bool*							m_pArrived = { nullptr };
	_bool							m_isPatrol = { true };

	_bool							m_isNeedMakeNewPath = { false };
	_bool							m_isNeedMakeNewPath_Detour_Static = { false };
	_bool							m_isNeedMakeNewPath_Detour_Dynamic = { false };
	_bool							m_isNeedMakeNewPath_All_Blocked_To_Goal = { false };

	_uint							m_iMoveIndex = {};

private:
	_float							m_fObstacleRecognizeDist = {};

private:		//	For.Time Out
	_float							m_fReserveLength = {};
	_float							m_fMaxWaitTime = { 0.f };
	_float							m_fAccWaitTime = {};
	_bool							m_isWait = { false };

	vector<_uint>					m_ReserveIndices;
	vector<_uint>					m_PreReserveIndices;
	vector<_uint>					m_SensoredCurIndices;
	vector<_uint>					m_SensoredNextIndices;

	array<_bool, static_cast<_uint>(VOXEL_ID::_END)>		m_Walkable_Types;

public:
	static void Set_ReserveSensortive(const _float fSensortive) { if (0.f > fSensortive) return; ms_fReserveSensortive = fSensortive; }
	static _float Get_ReserveSensortive() { return ms_fReserveSensortive; }

private:
	static _float					ms_fReserveSensortive;

public:
	static CAgent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END