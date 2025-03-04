#include "..\Public\Agent.h"
#include "GameInstance.h"

_float CAgent::ms_fReserveSensortive = { 0.5f };

CAgent::CAgent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CAgent::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	AGENT_DESC* pDesc = { static_cast<AGENT_DESC*>(pArg) };
	m_pTransform = pDesc->pTransform;
	m_pMove = pDesc->pMove;
	m_pArrived = pDesc->pArrived;
	m_pCommandInfoQueue = pDesc->pCommandInfoQueue;

	if (nullptr == m_pTransform ||
		nullptr == m_pMove ||
		nullptr == m_pArrived ||
		nullptr == m_pCommandInfoQueue)
		return E_FAIL;

	Safe_AddRef(m_pTransform);

	m_fSpeed = 3.f;
	m_fMinSpeed = 1.f;
	m_fMaxSpeed = 5.f;
	m_fRotatePerSec = 180.f;
	m_fObstacleRecognizeDist = 5.f;
	m_fMaxWaitTime = 0.25f;

	return S_OK;
}

void CAgent::Tick(_float fTimeDelta)
{
	Sense_Reserve();

	if (true == (*m_pMove))
	{
		if (true == m_isWait)
		{
			m_fAccWaitTime += fTimeDelta;
			if (m_fAccWaitTime >= m_fMaxWaitTime)
			{
				if (true == Is_All_BlockToGoal())
				{
					Check_NextPathes_All_Blocked();
				}

				else
				{
					Check_NextPathes_DynamicReserve();
				}

				m_fAccWaitTime = 0.f;
				m_isWait = false;
			}
		}

		else
		{
			Move_Seq(fTimeDelta);
			Check_NextPathes_StaticObstacle();
		}
	}
}

void CAgent::Move_Seq(_float fTimeDelta)
{
	_uint				iCurVoxelIndex;
	_float3				vPos;
	if (FAILED(Get_CurrentVoxelIndex(iCurVoxelIndex, m_iMoveIndex)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vPos)))
	{
		Stop_Move();
		return;
	}

	_float				fRemainTime = { fTimeDelta };
	while (true)
	{
		_float			fPreRemainTime = fRemainTime;

		Move_To_Target(fRemainTime, XMVectorSetW(XMLoadFloat3(&vPos), 1.f));

		_float			fUsedTime = fPreRemainTime - fRemainTime;
		Rotate_To_Target(fUsedTime, XMVectorSetW(XMLoadFloat3(&vPos), 1.f));


		if (0.f < fRemainTime)
		{
			if (FAILED(Get_NextVoxelIndex(iCurVoxelIndex, m_iMoveIndex)))
			{
				if (true == Is_Patrol())
				{
					m_iMoveIndex = 0;
					m_isNeedMakeNewPath = true;

					_uint		iStartIndex = { m_pCommandInfoQueue->front()->iStartIndex };
					_uint		iGoalIndex = { m_pCommandInfoQueue->front()->iGoalIndex };

					m_pCommandInfoQueue->front()->iStartIndex = iGoalIndex;
					m_pCommandInfoQueue->front()->iGoalIndex = iStartIndex;

					return;
				}

				else
				{
					--m_iMoveIndex;
					Stop_Move();
					return;
				}
			}

			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vPos)))
			{
				Stop_Move();
				return;
			}
		}

		else
			break;
	}
}

void CAgent::Move_To_Target(_float& fRemainTime, _fvector vTargetPos)
{
	if (nullptr == m_pTransform)
		return;

	_vector			vPosition = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
	_vector			vDir = { vTargetPos - vPosition };
	_vector			vUnitDir = { XMVector3Normalize(vDir) };
	_float			fDist = { XMVectorGetX(XMVector3Length(vDir)) };

	_float			fMoveDist = { m_fSpeed * fRemainTime };
	_vector			vMoveDir = { vUnitDir * fminf(fMoveDist, fDist) };
	_vector			vNewPos = { vPosition + vMoveDir };

	if (fMoveDist > fDist)
		fRemainTime -= (fDist / m_fSpeed);
	else
		fRemainTime = 0.f;

	m_pTransform->Set_State(CTransform::STATE_POSITION, vNewPos);
}

void CAgent::Rotate_To_Target(_float& fRemainTime, _fvector vTargetPos)
{
	if (nullptr == m_pTransform)
		return;

	_vector			vPosition = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
	_vector			vDir = { vTargetPos - vPosition };
	_vector			vDirXZPlane = { XMVectorSetY(vDir, 0.f) };
	_vector			vUnitDir = { XMVector3Normalize(vDirXZPlane) };
	_vector			vLook = { m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) };
	_vector			vUnitLook = { XMVector3Normalize(vLook) };

	if (XMVectorGetX(XMVector3Length(vDirXZPlane)) < EPSILON)
		return;

	_float			fDot = { XMVectorGetX(XMVector3Dot(vUnitDir, vUnitLook)) };
	if (fDot >= 1.f - EPSILON)		//	0도 ==>>> 방향성이 일치하는 벡터이므로 외적축이 나오지 않음
		return;

	_vector			vAxis;

	if (fDot >= -1.f + EPSILON)		//	180도 이하 이므로 정상 외적 가능
		vAxis = XMVector3Cross(vUnitLook, vUnitDir);
	else
		vAxis = XMVector3Normalize(m_pTransform->Get_State_Vector(CTransform::STATE_UP));

	vAxis = XMVector3Normalize(vAxis);

	_float			fAngle = { fminf(acosf(fDot), XMConvertToRadians(m_fRotatePerSec) * fRemainTime) };

	_matrix			RotateMatrix = { XMMatrixRotationAxis(vAxis, fAngle) };
	_matrix			WorldMatrix = { m_pTransform->Get_WorldMatrix() };

	WorldMatrix.r[CTransform::STATE_POSITION] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	WorldMatrix = WorldMatrix * RotateMatrix;
	WorldMatrix.r[CTransform::STATE_POSITION] = vPosition;

	m_pTransform->Set_WorldMatrix(WorldMatrix);
}

void CAgent::Check_NextPathes_StaticObstacle()
{
	vector<_uint>			NextVoxelIndices = { Compute_NextVoxelIndices_InRange(m_fObstacleRecognizeDist) };

	for (auto iNextIndex : NextVoxelIndices)
	{
		if (true == m_pGameInstance->Is_Exist_Voxel(iNextIndex, VOXEL_LAYER::_OBSTACLE_STATIC))
		{
			m_isNeedMakeNewPath_Detour_Static = true;
			return;
		}
	}
}

void CAgent::Check_NextPathes_DynamicReserve()
{
	m_isNeedMakeNewPath_Detour_Dynamic = true;
}

void CAgent::Check_NextPathes_All_Blocked()
{
	m_isNeedMakeNewPath_All_Blocked_To_Goal = true;
}


_bool CAgent::Is_All_BlockToGoal()
{
	if (true == m_pCommandInfoQueue->empty())
		return false;

	const auto& BestPathIndices = m_pCommandInfoQueue->front()->BestPathIndices;

	auto iterPath = BestPathIndices.begin();
	advance(iterPath, m_iMoveIndex);

	while (iterPath != BestPathIndices.end())
	{
		_uint			iNextIndex = (*iterPath);
		++iterPath;

		if ((false == m_pGameInstance->Is_Exist_Voxel(iNextIndex, VOXEL_LAYER::_OBSTACLE_STATIC) &&
			false == m_pGameInstance->Is_Exist_Voxel(iNextIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC)) ||
			m_ReserveIndices.end() != find(m_ReserveIndices.begin(), m_ReserveIndices.end(), iNextIndex))
			return false;
	}

	return true;
}

HRESULT CAgent::Sense_Reserve()
{
	Clear_ReserveIndices();
	Sense_CurPos();
	Sense_NextPoses();
	Reserve_SensoredIndices();

	return S_OK;
}

HRESULT CAgent::Clear_ReserveIndices()
{
	m_SensoredCurIndices.clear();
	m_SensoredNextIndices.clear();
	m_PreReserveIndices = move(m_ReserveIndices);

	return S_OK;
}

HRESULT CAgent::Sense_CurPos()
{
	_vector				vCurPosition = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
	_float3				vCurPositionFloat3;
	XMStoreFloat3(&vCurPositionFloat3, vCurPosition);

	//	이동중이아니거나 이동목록이 빈 경우
	if (false == *m_pMove ||
		true == m_pCommandInfoQueue->empty())
	{
		_uint					iCurVoxelIndex;
		if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurPositionFloat3, iCurVoxelIndex)))
			return E_FAIL;

		m_SensoredCurIndices.push_back(iCurVoxelIndex);
	}

	//	이동중인 경우
	else if(0 != m_iMoveIndex)	//	아직 첫 움직임 = > 이동중인 노드가 첫 노드
	{
		const auto&			CurPathIndices = m_pCommandInfoQueue->front()->BestPathIndices;
		auto				iterNext = CurPathIndices.begin();
		advance(iterNext, m_iMoveIndex - 1);
		auto				iterPre = (iterNext)++;

		const _uint			iPreVoxelIndex = { *(iterPre) };
		const _uint			iNextVoxelIndex = { *(iterNext) };

		_float3				vPreVoxelPosFloat3, vNextVoxelPosFloat3;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iPreVoxelIndex, vPreVoxelPosFloat3)) ||
			FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNextVoxelIndex, vNextVoxelPosFloat3)))
			return E_FAIL;

		_vector				vPreVoxelPos = { XMLoadFloat3(&vPreVoxelPosFloat3) };
		_vector				vNextVoxelPos = { XMLoadFloat3(&vNextVoxelPosFloat3) };

		const _float		fDistFromPre = { XMVectorGetX(XMVector3Length(vCurPosition - vPreVoxelPos)) };
		const _float		fDistToNext = { XMVectorGetX(XMVector3Length(vNextVoxelPos - vCurPosition)) };

		if (fDistFromPre < fDistToNext)
			m_SensoredCurIndices.push_back(iPreVoxelIndex);
	}

	return S_OK;
}

HRESULT CAgent::Sense_NextPoses()
{
	if (false == *m_pMove ||
		true == m_pCommandInfoQueue->empty())
		return S_OK;

	vector<_uint>			NextVoxelIndices = { Compute_NextVoxelIndices_InRange(m_fSpeed * ms_fReserveSensortive) };
	m_SensoredNextIndices = move(NextVoxelIndices);

	return S_OK;
}

HRESULT CAgent::Reserve_SensoredIndices()
{
	for (auto iSensoredIndex : m_SensoredCurIndices)
	{
		//	현재 내위치를 타 에이전트가 점유중인 경우
		if (m_PreReserveIndices.end() != find(m_PreReserveIndices.begin(), m_PreReserveIndices.end(), iSensoredIndex))
		{
			m_ReserveIndices.push_back(iSensoredIndex);		
		}
		else
		{
			if (true == m_pGameInstance->Is_Exist_Voxel(iSensoredIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC))
			{
				m_isWait = true;
				m_fAccWaitTime = 0.f;
				return S_OK;
			}

			else
			{
				if (FAILED(m_pGameInstance->Add_Voxel(iSensoredIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC, VOXEL_ID::_OBSTACLE, VOXEL_STATE::_IS_RESERVE)))
					return E_FAIL;

				m_ReserveIndices.push_back(iSensoredIndex);
			}
		}
	}

	_uint			iReserveNextCnt = {};
	for (auto iSensoredIndex : m_SensoredNextIndices)
	{
		if (m_PreReserveIndices.end() != find(m_PreReserveIndices.begin(), m_PreReserveIndices.end(), iSensoredIndex))
		{
			m_ReserveIndices.push_back(iSensoredIndex);
			++iReserveNextCnt;
			continue;
		}

		else
		{
			if (true == m_pGameInstance->Is_Exist_Voxel(iSensoredIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC))
				break;

			if (FAILED(m_pGameInstance->Add_Voxel(iSensoredIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC, VOXEL_ID::_OBSTACLE, VOXEL_STATE::_IS_RESERVE)))
				return E_FAIL;

			m_ReserveIndices.push_back(iSensoredIndex);
			++iReserveNextCnt;
		}
	}

	for (auto iPreReserveIndex : m_PreReserveIndices)
	{
		if (m_ReserveIndices.end() == find(m_ReserveIndices.begin(), m_ReserveIndices.end(), iPreReserveIndex) &&
			FAILED(m_pGameInstance->Erase_Voxel(iPreReserveIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC)))
			return E_FAIL;			
	}

	if (0 == iReserveNextCnt &&
		false == m_SensoredNextIndices.empty())
	{
		if (false == m_isWait)
			m_fAccWaitTime = 0.f;
		m_isWait = true;
	}
	else
		m_isWait = false;

	return S_OK;
}

void CAgent::Reserve_NextPathes()
{
	vector<_uint>			NextVoxelIndices = { Compute_NextVoxelIndices_InRange(m_fSpeed * ms_fReserveSensortive) };
	_vector					vCurPosition = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
	_float3					vCurPositionFloat3;
	XMStoreFloat3(&vCurPositionFloat3, vCurPosition);

	_uint					iCurVoxelIndex;
	if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurPositionFloat3, iCurVoxelIndex)))
		return;

	if (true == m_pGameInstance->Is_Exist_Voxel(iCurVoxelIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC) &&
		m_ReserveIndices.end() == find(m_ReserveIndices.begin(), m_ReserveIndices.end(), iCurVoxelIndex))
	{
		if (false == m_isWait)
			m_fAccWaitTime = 0.f;
		m_isWait = true;
		return;
	}

	reverse(NextVoxelIndices.begin(), NextVoxelIndices.end());
	if (NextVoxelIndices.end() == find(NextVoxelIndices.begin(), NextVoxelIndices.end(), iCurVoxelIndex))
		NextVoxelIndices.push_back(iCurVoxelIndex);
	reverse(NextVoxelIndices.begin(), NextVoxelIndices.end());

	if (true == NextVoxelIndices.empty())
		return;

	_uint		iBlockedCnt = {};
	//	새로운 전방의 인덱스들중 타 객체가 점유 한 복셀이 없는 복셀까지만 연속적으로 점유한다.
	for (auto iter = NextVoxelIndices.begin(); iter != NextVoxelIndices.end(); )
	{
		const _uint			iNextVoxelIndex = { *iter };
		if (m_ReserveIndices.end() != find(m_ReserveIndices.begin(), m_ReserveIndices.end(), iNextVoxelIndex))
		{
			++iter;
			continue;
		}
		else
		{
			if (true == m_pGameInstance->Is_Exist_Voxel(iNextVoxelIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC))
			{
				while (iter != NextVoxelIndices.end())
				{
					iter = NextVoxelIndices.erase(iter);
					++iBlockedCnt;
				}
			}

			else
			{
				m_pGameInstance->Add_Voxel(*iter, VOXEL_LAYER::_OBSTACLE_DYNAMIC, VOXEL_ID::_OBSTACLE, VOXEL_STATE::_IS_RESERVE);
				m_ReserveIndices.push_back(*iter);
				++iter;
			}
		}
	}

	//	더 이상 점유하지 않는 복셀 노드제거
	for (auto iter = m_ReserveIndices.begin(); iter != m_ReserveIndices.end(); )
	{
		const _uint			iReservedVoxelIndex = { *iter };
		if (NextVoxelIndices.end() == find(NextVoxelIndices.begin(), NextVoxelIndices.end(), iReservedVoxelIndex))
		{
			iter = m_ReserveIndices.erase(iter);
			m_pGameInstance->Erase_Voxel(iReservedVoxelIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC);
		}
		else
			++iter;
	}

	if (1 == m_ReserveIndices.size() &&
		0 < iBlockedCnt)
	{
		if (false == m_isWait)
			m_fAccWaitTime = 0.f;

		m_isWait = true;
	}
	else
		m_isWait = false;
}

void CAgent::Update_CurReserveIndices()
{
	_vector		vCurPos = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };

	_uint		iNextPathIndex = { m_iMoveIndex };
	_uint		iNextVoxelIndex = {};
	_float		fAccDist = {};
	_float3		vNextVoxelPosFloat3;

	if (!(FAILED(Get_CurrentVoxelIndex(iNextVoxelIndex, iNextPathIndex))))
	{
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNextVoxelIndex, vNextVoxelPosFloat3)))
			return;

		fAccDist += XMVectorGetX(XMVector3Length(XMLoadFloat3(&vNextVoxelPosFloat3) - vCurPos));


		while (!(FAILED(Get_NextVoxelIndex(iNextVoxelIndex, iNextPathIndex))))
		{

		}
	}

}

vector<_uint> CAgent::Compute_NextVoxelIndices_InRange(const _float fDist)
{
	if (true == m_pCommandInfoQueue->empty())
		return vector<_uint>();

	vector<_uint>		NextVoxelIndicies;

	_vector			vCurPosition = { m_pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
	_vector			vPrePosition;

	list<_uint>& BestIndices = { m_pCommandInfoQueue->front()->BestPathIndices };

	_float			fMaxDist = { fDist };
	_float			fAccDist = {};
	auto			iter{ BestIndices.begin() };
	advance(iter, m_iMoveIndex);

	while (iter != BestIndices.end())
	{
		vPrePosition = vCurPosition;

		_uint			iNextIndex = { *iter };
		_float3			vNextWorldPos;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNextIndex, vNextWorldPos)))
			break;

		vCurPosition = XMVectorSetW(XMLoadFloat3(&vNextWorldPos), 1.f);

		fAccDist += XMVectorGetX(XMVector3Length(vCurPosition - vPrePosition));

		if (fMaxDist <= fAccDist)
			break;

		NextVoxelIndicies.push_back(iNextIndex);
		++iter;
	}

	return NextVoxelIndicies;
}

_bool CAgent::Is_NeedMakeNewPath()
{
	const _bool		isNeed = { m_isNeedMakeNewPath };
	m_isNeedMakeNewPath = false;

	return isNeed;
}

_bool CAgent::Is_NeedMakeNewPath_Detour_Static()
{
	const _bool		isNeed = { m_isNeedMakeNewPath_Detour_Static };
	m_isNeedMakeNewPath_Detour_Static = false;

	return isNeed;
}

_bool CAgent::Is_NeedMakeNewPath_Detour_Dynamic()
{
	const _bool		isNeed = { m_isNeedMakeNewPath_Detour_Dynamic };
	m_isNeedMakeNewPath_Detour_Dynamic = false;

	return isNeed;
}

_bool CAgent::Is_NeedMakeNewPath_All_Blocked_To_Goal()
{
	const _bool		isNeed = { m_isNeedMakeNewPath_All_Blocked_To_Goal };
	m_isNeedMakeNewPath_All_Blocked_To_Goal = false;

	return isNeed;
}

void CAgent::Start_Move()
{
	m_iMoveIndex = 0;

	if (true == m_pCommandInfoQueue->empty())
		return;

	list<_uint>& BestIndices = { m_pCommandInfoQueue->front()->BestPathIndices };

	if (true == BestIndices.empty())
		return;

	_uint				iStartIndex = { BestIndices.front() };
	_float3				vStartPos;
	m_pGameInstance->Get_WorldPosition_Voxel(iStartIndex, vStartPos);
	m_pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMLoadFloat3(&vStartPos), 1.f));
}

void CAgent::Start_Wait()
{
	m_fAccWaitTime = 0.f;
	m_isWait = true;
}

void CAgent::Stop_Move()
{
	m_iMoveIndex = 0;
}

void CAgent::Set_MakeNewPass()
{
	m_isNeedMakeNewPath = true;
}

HRESULT CAgent::Get_CurrentVoxelIndex(_uint& iCurVoxelIndex, const _uint iPathIndex)
{
	if (true == m_pCommandInfoQueue->empty())
		return E_FAIL;

	list<_uint>& BestIndices = { m_pCommandInfoQueue->front()->BestPathIndices };

	if (static_cast<_uint>(BestIndices.size()) <= iPathIndex)
		return E_FAIL;

	auto iter{ BestIndices.begin() };
	advance(iter, iPathIndex);
	iCurVoxelIndex = *iter;

	return S_OK;
}

HRESULT CAgent::Get_NextVoxelIndex(_uint& iNextVoxelIndex, _uint& iPathIndex)
{
	return Get_CurrentVoxelIndex(iNextVoxelIndex, ++iPathIndex);
}

CAgent* CAgent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CAgent* pInstance = { new CAgent(pDevice, pContext) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CAgent"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAgent::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pTransform);

	for (auto iReserveIndex : m_ReserveIndices)
	{
		if (FAILED(m_pGameInstance->Erase_Voxel(iReserveIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC)))
		{
			MSG_BOX(TEXT("Faield To Erase Voxel Agent Free "));
		}
	}
}
