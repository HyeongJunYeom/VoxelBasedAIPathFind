#include "..\Public\Navigation.h"
#include "GameInstance.h"

#include "PathFinder_Voxel.h"
#include "Agent.h"
#include "Boid.h"
#include "Formation.h"

//	#define BOID_FLOCKING

const _float	CNavigation::ms_fMaxCostDetour = { 5.f };

CNavigation::CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CNavigation::CNavigation(const CNavigation& rhs)
	: CComponent{ rhs }
{
}

HRESULT CNavigation::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CNavigation::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	NAVIGATION_DESC* pDesc = { static_cast<NAVIGATION_DESC*>(pArg) };
	m_pTransformCom = pDesc->pTransform;
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	Safe_AddRef(m_pTransformCom);

	if (FAILED(Add_Supports()))
		return E_FAIL;

	return S_OK;
}

void CNavigation::Priority_Tick(_float fTimeDelta)
{
}

void CNavigation::Tick(_float fTimeDelta)
{
	if (nullptr == m_pAgent)
		return;

	_vector			vPosition = { m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) };
	_float3			vPositionFloat3;
	XMStoreFloat3(&vPositionFloat3, vPosition);

	VOXEL_ID		eVoxelID = { VOXEL_ID::_END };
	if (FAILED(m_pGameInstance->Get_VoxelID(vPositionFloat3, eVoxelID, VOXEL_LAYER::_STATIC)))
	{
		m_isFall = true;
		m_fAccFallSpeed += 9.8f * fTimeDelta;
	}
	else
	{
		m_isFall = false;
		m_fAccFallSpeed = 0.f;
	}

	_vector			vFallDir = { XMVectorSet(0.f, -m_fAccFallSpeed * fTimeDelta, 0.f, 0.f) };
	_vector			vNewPosition = { vPosition + vFallDir };
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vNewPosition);

	XMStoreFloat3(&vPositionFloat3, vNewPosition);


	PATH_MODE		eCurPathMode = { PATH_MODE::_END };

	if (false == m_CommandInfoQueue.empty())
		eCurPathMode = m_CommandInfoQueue.front()->ePathMode;


#pragma region OLD MOVE

#ifdef BOID_FLOCKING
	if (PATH_MODE::_MULTI_AGENT == eCurPathMode)
#else
	if (false)
#endif // BOID_FLOCKING
	{
		COMMAND_INFO_MAPF*		pCommandInfo = { static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.front()) };
		const _uint				iFlowFieldIndexTag = { pCommandInfo->iFlowIndex };

		if (true == m_isMove)
		{
			m_pBoid->Set_CurFlowFieldIndex(iFlowFieldIndexTag);

			_uint			iVoxelIndex;
			if (!(FAILED(m_pGameInstance->Get_Index_Voxel(vPositionFloat3, iVoxelIndex))))
			{
				_float			fDistFromGoal = { m_pGameInstance->Get_Distance_From_Goal_FlowField(iVoxelIndex, iFlowFieldIndexTag) };
				if (0.15f > fDistFromGoal)
					m_isArrived = true;

				else
				{
					_uint			iParentVoxelIndex;
					if (!(FAILED(m_pGameInstance->Get_ParentIndex_FlowField(vPositionFloat3, iFlowFieldIndexTag, iParentVoxelIndex))))
					{
						_float3			vNextPos;
						m_pGameInstance->Get_WorldPosition_Voxel(iParentVoxelIndex, vNextPos);

						m_pBoid->Set_NextPos(vNextPos);
					}
				}
			}			
		}

		if (true == m_isArrived)
		{
			COMMAND_INFO_MAPF*		pCommandInfo = { static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.front()) };
			const _uint				iFlowFieldIndexTag = { pCommandInfo->iFlowIndex };

			if (FAILED(m_pGameInstance->Release_FlowField(iFlowFieldIndexTag)))
			{
				MSG_BOX(TEXT("Release Failed Flow Field"));
			}

			m_CommandInfoQueue.erase(m_CommandInfoQueue.begin());

			if (true == m_CommandInfoQueue.empty())
			{
				m_pBoid->Set_NextPos(vPositionFloat3);
				m_isMove = false;
			}

			else
			{
				COMMAND_INFO_MAPF*		pCommandInfo = { static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.front()) };
				const _uint				iNewFlowFieldIndexTag = { pCommandInfo->iFlowIndex };

				m_pBoid->Set_CurFlowFieldIndex(iNewFlowFieldIndexTag);

				_uint			iParentVoxelIndex;
				if (!(FAILED(m_pGameInstance->Get_ParentIndex_FlowField(vPositionFloat3, iNewFlowFieldIndexTag, iParentVoxelIndex))))
				{
					_float3			vNextPos;
					m_pGameInstance->Get_WorldPosition_Voxel(iParentVoxelIndex, vNextPos);

					m_pBoid->Set_NextPos(vNextPos);
					m_isMove = true;
				}
				else
				{
					m_pBoid->Set_NextPos(vPositionFloat3);
					m_isMove = false;
				}
			}

			m_isArrived = false;
		}
	}


	else if (PATH_MODE::_SINGLE_AGENT == eCurPathMode)
	{
		/*COMMAND_INFO_SAPF*		pCommandInfo = { static_cast<COMMAND_INFO_SAPF*>(m_CommandInfoQueue.front()) };

		if (true == m_isMove)
		{
			_uint			iVoxelIndex;
			if (!(FAILED(m_pGameInstance->Get_Index_Voxel(vPositionFloat3, iVoxelIndex))))
			{
				_float			fDistFromGoal = {};
				if (FAILED(Compute_Distance_From_Goal(m_CommandInfoQueue.front()->iGoalIndex, m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fDistFromGoal)))
					return;

				if (0.25f > fDistFromGoal)
					m_isArrived = true;

				else
				{
					_uint			iParentVoxelIndex;
					if (!(FAILED(m_pGameInstance->Get_ParentIndex_FlowField(vPositionFloat3, iFlowFieldIndexTag, iParentVoxelIndex))))
					{
						_float3			vNextPos;
						m_pGameInstance->Get_WorldPosition_Voxel(iParentVoxelIndex, vNextPos);

						m_pBoid->Set_NextPos(vNextPos);
					}
				}
			}
		}*/
	}


#ifdef BOID_FLOCKING
	if (PATH_MODE::_MULTI_AGENT == eCurPathMode)
#else
	if (false)
#endif
	{
		_vector			vCurPos = { m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) };
		_float3			vCurPosFloat3;

		XMStoreFloat3(&vCurPosFloat3, vCurPos);

		_float3			vBoidResultDirFloat3;
		if (FAILED(m_pBoid->Compute_ResultDirection_BoidMove(vPosition, vBoidResultDirFloat3)))
			return;

		_float3			vTargetPosFormation = { m_pFormation->Get_TargetPos() };
		_vector			vDirectionToTarget = { XMLoadFloat3(&vTargetPosFormation) - vCurPos };

		_vector			vResultDirection = { XMVector3Normalize(XMLoadFloat3(&vBoidResultDirFloat3) /*+ vDirectionToTarget*/) };
		const _float	fAgentSpeed = { m_pAgent->Get_Speed() };

		_vector			vMoveDirection = { vResultDirection * fAgentSpeed * fTimeDelta };

		_vector			vResultPosition = { vCurPos + vMoveDirection };

		_uint3			vCurIndexPos;
		m_pGameInstance->Get_IndexPosition_Voxel(vCurPosFloat3, vCurIndexPos);

		_float3			vRecoveredDir{};
		Recover_Wall(vCurPos, vMoveDirection, m_pAgent->Get_Radius(), vRecoveredDir);


		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vCurPos + XMVector3Normalize(XMLoadFloat3(&vRecoveredDir)) * m_pAgent->Get_Speed() * fTimeDelta);
	}
#pragma endregion

	//	기존 방식
#ifdef BOID_FLOCKING
	else if(PATH_MODE::_SINGLE_AGENT == eCurPathMode)
#else
	else if(true)
#endif
	{ 
		m_pAgent->Tick(fTimeDelta);

		/*if (true == m_pAgent->Is_NeedMakeNewPath())
		{
			list<_uint>			BestPathIndcies;
			NODE_INFOS			NodeInfos;
			const _uint			iStartIndex = { m_CommandInfoQueue.front()->iStartIndex };
			const _uint			iGoalIndex = { m_CommandInfoQueue.front()->iGoalIndex };

			if (FAILED(m_pGameInstance->Make_Path_Voxel(BestPathIndcies, NodeInfos, iStartIndex, { iGoalIndex }, false, false, {})))
				return;

			COMMAND_INFO_SAPF*		pCommandInfo = { static_cast<COMMAND_INFO_SAPF*>(m_CommandInfoQueue.front()) };
			pCommandInfo->BestPathIndices = move(BestPathIndcies);
		}*/
		if (true == m_pAgent->Is_NeedMakeNewPath_Detour_Static())
		{
			list<_uint>			PrePathIndices;
			NODE_INFOS			NewNodeInfos;
			_uint				iNewStartIndex;
			vector<_uint>		NewGoals;

			COMMAND_INFO_SAPF*	pCommandInfo = { static_cast<COMMAND_INFO_SAPF*>(m_CommandInfoQueue.front()) };
			list<_uint>			BestPathIndices = { pCommandInfo->BestPathIndices };

			if (FAILED(Ready_Detour_Static_Obstacle(BestPathIndices, PrePathIndices, m_pAgent->Get_MoveIndex(), iNewStartIndex, NewGoals, {})))
				return;

			list<_uint>			NewPathIndices;

			if (FAILED(m_pGameInstance->Make_Path_Voxel(NewPathIndices, NewNodeInfos, iNewStartIndex, NewGoals, true, false, {})))
				return;

			if (FAILED(End_Detour()))
				return;

			BestPathIndices = move(PrePathIndices);
			for (auto iNewIndex : NewPathIndices)
				BestPathIndices.push_back(iNewIndex);
			auto iter{ find(NewGoals.begin(), NewGoals.end(), BestPathIndices.back()) };
			++iter;
			while (iter != NewGoals.end())
			{
				BestPathIndices.push_back(*iter);
				++iter;
			}

			pCommandInfo->BestPathIndices = move(BestPathIndices);
		}

		else if (true == m_pAgent->Is_NeedMakeNewPath_Detour_Dynamic())
		{
			list<_uint>			PrePathIndices;
			NODE_INFOS			NewNodeInfos;
			_uint				iNewStartIndex;
			vector<_uint>		NewGoals;

			vector<_uint>		MyReserveIndices = { m_pAgent->Get_ReserveIndices() };

			COMMAND_INFO_SAPF*	pCommandInfo = { static_cast<COMMAND_INFO_SAPF*>(m_CommandInfoQueue.front()) };
			list<_uint>			BestPathIndices = { pCommandInfo->BestPathIndices };

			if (FAILED(Ready_Detour_Dynamic_Obstacle(BestPathIndices, PrePathIndices, m_pAgent->Get_MoveIndex(), iNewStartIndex, NewGoals, MyReserveIndices)))
					return;

			list<_uint>			NewPathIndices;

			m_pGameInstance->Set_MaxCost(ms_fMaxCostDetour);
			if (FAILED(m_pGameInstance->Make_Path_Voxel(NewPathIndices, NewNodeInfos, iNewStartIndex, NewGoals, true, true, MyReserveIndices)))
			{
				m_pAgent->Start_Wait();
				return;
			}

			if (FAILED(End_Detour()))
				return;

			BestPathIndices = move(PrePathIndices);
			for (auto iNewIndex : NewPathIndices)
				BestPathIndices.push_back(iNewIndex);
			auto iter{ find(NewGoals.begin(), NewGoals.end(), BestPathIndices.back()) };
			++iter;
			while (iter != NewGoals.end())
			{
				BestPathIndices.push_back(*iter);
				++iter;
			}

			pCommandInfo->BestPathIndices = move(BestPathIndices);
			//	pCommandInfo->BestPathIndices = BestPathIndices;

			/*const _uint			iMoveIndex = { m_pAgent->Get_MoveIndex() };
			if (m_CommandInfoQueue.front()->BestPathIndices.size() <= iMoveIndex)
				int iA = 0;*/
		}

		else if (true == m_pAgent->Is_NeedMakeNewPath_All_Blocked_To_Goal())
		{
			auto start = std::chrono::high_resolution_clock::now();

			const _uint			iOriginGoalIndex = { m_CommandInfoQueue.front()->iGoalIndex };
			_float3				vOriginGoalPosFloat3;
			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iOriginGoalIndex, vOriginGoalPosFloat3)))
				return;

			_vector				vOriginGoalPos = { XMLoadFloat3(&vOriginGoalPosFloat3) };

			auto&				CurBestPath = m_CommandInfoQueue.front()->BestPathIndices;
			const _uint			iMoveIndex = { m_pAgent->Get_MoveIndex() };
			auto				iter = CurBestPath.begin();
			advance(iter, iMoveIndex > 0 ? iMoveIndex - 1 : 0);
			const _uint			iCurStartIndex = { *iter };

			const auto&			MyReserveIndices = m_pAgent->Get_ReserveIndices();

			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
			//	BFS 로직 

			queue<_uint>				BFSQueue;
			unordered_set<_uint>		VisitedSet;
			BFSQueue.push(iOriginGoalIndex);
			VisitedSet.insert(iOriginGoalIndex);

			_uint						iResultGoalIndex = { iOriginGoalIndex };
			unordered_set<_uint>		TempGoalIndicesSet;
			vector<_uint>				ResultGoalIndices;

			while (false == BFSQueue.empty())
			{
				const _uint			iCurIndex = { BFSQueue.front() };
				BFSQueue.pop();

				/*auto				NeighborIndices = m_pGameInstance->Get_NeighborIndices_Voxel(iCurGoalIndex, VOXEL_LAYER::_STATIC);
				_bool				isBlocked = { true };
				for (const auto iNeighborIndex : NeighborIndices)
				{
					VOXEL_ID			eStaticID;
					if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eStaticID, VOXEL_LAYER::_STATIC)))
						return;
					if (eStaticID != VOXEL_ID::_FLOOR)
						continue;
					if (true == m_pGameInstance->Is_Exist_Voxel(iNeighborIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC) &&
						MyReserveIndices.end() == find(MyReserveIndices.begin(), MyReserveIndices.end(), iNeighborIndex))
						continue;

					isBlocked = false;
					break;
				}
								if(true == isBlocked)
					continue;
				*/

				if (false == m_pGameInstance->Is_Exist_Voxel(iCurIndex, VOXEL_LAYER::_OBSTACLE_STATIC) &&
					(false == m_pGameInstance->Is_Exist_Voxel(iCurIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC) ||
					MyReserveIndices.end() != find(MyReserveIndices.begin(), MyReserveIndices.end(), iCurIndex)))
					TempGoalIndicesSet.insert(iCurIndex);

				auto				NeighborIndices = m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC);
				if (iCurIndex == iCurStartIndex)
				{
					for (auto iNeighborIndex : NeighborIndices)
					{
						if (VisitedSet.end() != VisitedSet.find(iNeighborIndex))
							continue;

						if (false == m_pGameInstance->Is_Exist_Voxel(iNeighborIndex, VOXEL_LAYER::_OBSTACLE_STATIC) &&
							(false == m_pGameInstance->Is_Exist_Voxel(iNeighborIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC) ||
								MyReserveIndices.end() != find(MyReserveIndices.begin(), MyReserveIndices.end(), iNeighborIndex)))
							TempGoalIndicesSet.insert(iNeighborIndex);
					}

					break;
				}

				else
				{
					for (const auto iNeighborIndex : NeighborIndices)
					{
						if (VisitedSet.end() != VisitedSet.find(iNeighborIndex))
							continue;

						VisitedSet.insert(iNeighborIndex);

						VOXEL_ID			eID;
						if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eID, VOXEL_LAYER::_STATIC)) ||
							VOXEL_ID::_FLOOR != eID)
							continue;

						BFSQueue.push(iNeighborIndex);
					}
				}
			}

			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////

			//	탐색 가능 노드만 컬링
			queue<_uint>				BFSQueue_Culled;
			unordered_set<_uint>		Visited_Culled;
			BFSQueue_Culled.push(iCurStartIndex);
			Visited_Culled.insert(iCurStartIndex);
			ResultGoalIndices.push_back(iCurStartIndex);
			
			while (false == BFSQueue_Culled.empty())
			{
				const _uint		iCurIndex = { BFSQueue_Culled.front() };
				BFSQueue_Culled.pop();
				
				auto			NeighborIndices = m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC);

				for (auto iNeighborIndex : NeighborIndices)
				{
					if (Visited_Culled.end() != Visited_Culled.find(iNeighborIndex))
						continue;

					Visited_Culled.insert(iNeighborIndex);

					if (TempGoalIndicesSet.end() == TempGoalIndicesSet.find(iNeighborIndex))
						continue;

					BFSQueue_Culled.push(iNeighborIndex);
					ResultGoalIndices.push_back(iNeighborIndex);
				}
			}
			

			_float			fNearDist = { FLT_MAX };
			for (auto iIndex : ResultGoalIndices)
			{
				_float3			vResultPos;
				if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iIndex, vResultPos)))
					continue;

				_vector			vDir = { vOriginGoalPos - XMLoadFloat3(&vResultPos) };
				_float			fDist = { XMVectorGetX(XMVector3Length(vDir)) };

				if (fNearDist > fDist &&
					iResultGoalIndex != iIndex)
				{
					fNearDist = fDist;
					iResultGoalIndex = iIndex;
				}
			}

			_uint		iCurIndex;
			m_pGameInstance->Get_Index_Voxel(vPositionFloat3, iCurIndex);
				
			if (iResultGoalIndex == iCurIndex)
			{
				Next_Move();
			}

			else if (iResultGoalIndex != iOriginGoalIndex)
			{
				list<_uint>			OriginBestList = { m_CommandInfoQueue.front()->BestPathIndices };
				const _uint			iMoveIndex = { m_pAgent->Get_MoveIndex() };

				_uint iSizeOrigin = { static_cast<_uint>(OriginBestList.size()) };

				if (OriginBestList.size() <= iMoveIndex)
					int iA = 0;

				auto iter{ OriginBestList.begin() };
				advance(iter, iMoveIndex > 0 ? iMoveIndex - 1 : iMoveIndex);

				const _uint			iNewStartIndex = { *iter };
				const _uint			iNewGoalIndex = { iResultGoalIndex };

				if (iter == OriginBestList.end())
					return;

				while (iter != OriginBestList.end())
					iter = OriginBestList.erase(iter);

				list<_uint>			NewBestList;
				NODE_INFOS			NodeInfos;

				if (iNewGoalIndex == iNewStartIndex)
				{
					OriginBestList.push_back(iNewGoalIndex);

					m_CommandInfoQueue.front()->BestPathIndices = OriginBestList;
					m_CommandInfoQueue.front()->iGoalIndex = iNewGoalIndex;
				}
				
				else if (!(FAILED(m_pGameInstance->Make_Path_Voxel(NewBestList, NodeInfos, iNewStartIndex, { iNewGoalIndex }, true, true, MyReserveIndices))))
				{
					for (auto iIndex : NewBestList)
						OriginBestList.push_back(iIndex);

					m_CommandInfoQueue.front()->iGoalIndex = iResultGoalIndex;
					m_CommandInfoQueue.front()->BestPathIndices = move(OriginBestList);

					/*m_CommandInfoQueue.front()->BestPathIndices = OriginBestList;
					if (m_CommandInfoQueue.front()->BestPathIndices.size() <= iMoveIndex)
						int ia = 0;*/
				}
			}

			else
			{
				Next_Move();
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			cout << "Make New Goal Time " << duration.count() << endl;
		}

		if (true == m_isMove)
		{
			_uint		iGoalIndex = { m_CommandInfoQueue.front()->iGoalIndex };
			_float3		vPositionGoalFloat3;
			m_pGameInstance->Get_WorldPosition_Voxel(iGoalIndex, vPositionGoalFloat3);

			_vector		vPositionGoal = { XMLoadFloat3(&vPositionGoalFloat3) };
			_float		fDistGoal = { XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - vPositionGoal)) };

			if (0.15f >= fDistGoal)
			{
				Next_Move();
			}
		}

	}
}

void CNavigation::Late_Tick(_float fTimeDelta)
{
}

void CNavigation::Start_Move()
{
	if (nullptr == m_pAgent)
		return;

	m_isMove = true;
	m_pAgent->Start_Move();
}

void CNavigation::Stop_Move()
{
	if (nullptr == m_pAgent)
		return;

	m_isMove = false;
	m_pAgent->Stop_Move();
}

HRESULT CNavigation::Add_Supports()
{
	CAgent::AGENT_DESC			AgentDesc;
	AgentDesc.pTransform = m_pTransformCom;
	AgentDesc.pMove = &m_isMove;
	AgentDesc.pArrived = &m_isArrived;
	AgentDesc.pCommandInfoQueue = &m_CommandInfoQueue;

	m_pAgent = CAgent::Create(m_pDevice, m_pContext, &AgentDesc);
	if (nullptr == m_pAgent)
		return E_FAIL;

	CBoid::BOID_DESC			BoidDesc;
	BoidDesc.pTransform = m_pTransformCom;
	BoidDesc.pMove = &m_isMove;
	BoidDesc.pArrived = &m_isArrived;

	m_pBoid = CBoid::Create(m_pDevice, m_pContext, &BoidDesc);
	if (nullptr == m_pBoid)
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Boid(m_pBoid)))
		return E_FAIL;

	CFormation::FORMATION_DESC	FormationDesc;
	FormationDesc.pTransform = m_pTransformCom;
	FormationDesc.pSpeed = m_pAgent->Get_Speed_Ptr();
	FormationDesc.pMinSpeed = m_pAgent->Get_MinSpeed_Ptr();
	FormationDesc.pMaxSpeed = m_pAgent->Get_MaxSpeed_Ptr();
	FormationDesc.pMaxWaitTime = m_pAgent->Get_MaxWaitTime_Ptr();

	m_pFormation = CFormation::Create(m_pDevice, m_pContext, &FormationDesc);
	if (nullptr == m_pFormation)
		return E_FAIL;

	return S_OK;
}

void CNavigation::Set_MakeNewPass()
{
	m_pAgent->Set_MakeNewPass();
}

HRESULT CNavigation::Get_StartIndex(_uint& iIndex)
{
	if (true == m_CommandInfoQueue.empty())
		return E_FAIL;

	iIndex = m_CommandInfoQueue.front()->iStartIndex;
	return S_OK;
}

HRESULT CNavigation::Get_GoalIndex(_uint& iIndex)
{
	if (true == m_CommandInfoQueue.empty())
		return E_FAIL;

	iIndex = m_CommandInfoQueue.front()->iGoalIndex;
	return S_OK;
}

HRESULT CNavigation::Get_LastGoalIndex(_uint& iIndex)
{
	if (true == m_CommandInfoQueue.empty())
		return E_FAIL;

	iIndex = m_CommandInfoQueue.back()->iGoalIndex;
	return S_OK;
}

HRESULT CNavigation::Get_LastStartIndex(_uint& iIndex)
{
	if (true == m_CommandInfoQueue.empty())
		return E_FAIL;

	iIndex = m_CommandInfoQueue.back()->iStartIndex;
	return S_OK;
}

HRESULT CNavigation::Get_CurPos_Index(_uint& iIndex)
{
	_vector			vPosition = { m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) };
	_float3			vPositionFloat3;

	XMStoreFloat3(&vPositionFloat3, vPosition);

	if (FAILED(m_pGameInstance->Get_Index_Voxel(vPositionFloat3, iIndex)))
		return E_FAIL;

	return S_OK;
}

list<_float3> CNavigation::Get_BestPath_Positions_Float()
{
	list<_float3>		Positions;
	_float3				vPosition;

	if (true == m_CommandInfoQueue.empty())
		return Positions;

	list<_uint>&		BestIndices = { m_CommandInfoQueue.front()->BestPathIndices };

	for (auto iIndex : BestIndices)
	{
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iIndex, vPosition)))
		{
			cout << "list<_float3> CPathFinder_Voxel::Get_BestPath_Positions_Float()" << endl;
			cout << "if (FAILED(m_pGameInstance->Get_Position_Voxel(iIndex, vPosition)))" << endl;
		}

		Positions.push_back(vPosition);
	}

	return Positions;
}

list<_float3> CNavigation::Get_BestPath_Positions_Float_Optimized()
{
	return m_pGameInstance->Get_BestPath_Positions_Float_Optimized();
}

const list<_uint>& CNavigation::Get_BestPath_Indices()
{
	static list<_uint> Temp;
	if (true == m_CommandInfoQueue.empty())
		return Temp;

	return m_CommandInfoQueue.front()->BestPathIndices;
}

const list<_uint>& CNavigation::Get_BestPath_Indices_Optimized()
{
	return m_pGameInstance->Get_BestPath_Indices_Optimized();
}

const vector<_float3>& CNavigation::Get_BestPath_Positions_Smooth()
{
	return m_pGameInstance->Get_BestPath_Positions_Smooth();
}

const unordered_set<_uint>& CNavigation::Get_OpenList()
{
	return m_pGameInstance->Get_OpenList();
}

const unordered_set<_uint>& CNavigation::Get_CloseList()
{
	return m_pGameInstance->Get_CloseList();
}

ALGORITHM CNavigation::Get_Algorithm()
{
	return m_pGameInstance->Get_Algorithm();
}

MAIN_HEURISTIC CNavigation::Get_MainHeuristic()
{
	return m_pGameInstance->Get_MainHeuristic();
}

void CNavigation::Set_Algorithm(ALGORITHM eAlgorithm)
{
	return m_pGameInstance->Set_Algorithm(eAlgorithm);
}

void CNavigation::Set_MainHeuristic(MAIN_HEURISTIC eHeuristic)
{
	return m_pGameInstance->Set_MainHeuristic(eHeuristic);
}

TEST_MODE CNavigation::Get_OptimizerTestMode()
{
	return m_pGameInstance->Get_OptimizerTestMode();
}

void CNavigation::Set_OptimizerTestMode(TEST_MODE eTestMode)
{
	return m_pGameInstance->Set_OptimizerTestMode(eTestMode);
}

_float CNavigation::Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType)
{
	return m_pGameInstance->Get_HeuristicWeights(eHeuriticType);
}

void CNavigation::Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight)
{
	return m_pGameInstance->Set_HeuristicWeights(eHeuriticType, fWeight);
}

_float CNavigation::Compute_Total_Dist_BestPath()
{
	if (true == m_CommandInfoQueue.empty())
		return 0.f;

	return m_pGameInstance->Compute_Total_Dist_BestPath(m_CommandInfoQueue.front()->BestPathIndices);
}

_float CNavigation::Compute_Total_Dist_OptimizedPath()
{
	return m_pGameInstance->Compute_Total_Dist_OptimizedPath();
}

_llong CNavigation::Get_Make_Path_Time()
{
	return m_pGameInstance->Get_Make_Path_Time();
}

_llong CNavigation::Get_Optimize_Path_Time()
{
	return m_pGameInstance->Get_Optimize_Path_Time();
}

_llong CNavigation::Get_Smooth_Path_Time()
{
	return m_pGameInstance->Get_Smooth_Path_Time();
}

HRESULT CNavigation::Get_Cur_FlowFieldIndexTag(_uint& iIndexTag)
{
	if (true == m_CommandInfoQueue.empty() ||
		PATH_MODE::_MULTI_AGENT != m_CommandInfoQueue.front()->ePathMode)
		return E_FAIL;

	COMMAND_INFO_MAPF*		pCommandInfo = { static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.front()) };
	iIndexTag = pCommandInfo->iFlowIndex;
	return S_OK;
}

HRESULT CNavigation::Clear_CommandQueue()
{
	while (false == m_CommandInfoQueue.empty())
	{
		if (PATH_MODE::_MULTI_AGENT == m_CommandInfoQueue.back()->ePathMode)
		{
			COMMAND_INFO_MAPF* pCommandInfo = { static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.back()) };
			if (FAILED(m_pGameInstance->Release_FlowField(pCommandInfo->iFlowIndex)))
				return E_FAIL;
		}

		else if (PATH_MODE::_FORMATION == m_CommandInfoQueue.back()->ePathMode)
		{
			COMMAND_INFO_FORMATION* pCommandInfo = { static_cast<COMMAND_INFO_FORMATION*>(m_CommandInfoQueue.back()) };
			if (FAILED(m_pGameInstance->Erase_Formation(m_pFormation, pCommandInfo->iFormationIndex)))
				return E_FAIL;
		}

		Safe_Delete(m_CommandInfoQueue.back());
		m_CommandInfoQueue.pop_back();
	}	

	Stop_Move();

	return S_OK;
}

void CNavigation::Add_CommandInfo(COMMAND_INFO& CommandInfo)
{
	if (PATH_MODE::_END <= CommandInfo.ePathMode)
		return;

	if (CommandInfo.ePathMode == PATH_MODE::_MULTI_AGENT)
	{
		m_CommandInfoQueue.push_back(new COMMAND_INFO_MAPF());

		m_CommandInfoQueue.back()->BestPathIndices = move(CommandInfo.BestPathIndices);
		m_CommandInfoQueue.back()->ePathMode = CommandInfo.ePathMode;
		m_CommandInfoQueue.back()->iGoalIndex = CommandInfo.iGoalIndex;
		m_CommandInfoQueue.back()->iStartIndex = CommandInfo.iStartIndex;
		static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.back())->iFlowIndex = static_cast<COMMAND_INFO_MAPF*>(&CommandInfo)->iFlowIndex;
	}
	else if (CommandInfo.ePathMode == PATH_MODE::_SINGLE_AGENT)
	{
		m_CommandInfoQueue.push_back(new COMMAND_INFO_SAPF());

		m_CommandInfoQueue.back()->BestPathIndices = move(CommandInfo.BestPathIndices);
		m_CommandInfoQueue.back()->ePathMode = CommandInfo.ePathMode;
		m_CommandInfoQueue.back()->iGoalIndex = CommandInfo.iGoalIndex;
		m_CommandInfoQueue.back()->iStartIndex = CommandInfo.iStartIndex;
	}
	else if (CommandInfo.ePathMode == PATH_MODE::_FORMATION)
	{
		m_CommandInfoQueue.push_back(new COMMAND_INFO_FORMATION());

		m_CommandInfoQueue.back()->BestPathIndices = move(CommandInfo.BestPathIndices);
		m_CommandInfoQueue.back()->ePathMode = CommandInfo.ePathMode;
		m_CommandInfoQueue.back()->iGoalIndex = CommandInfo.iGoalIndex;
		m_CommandInfoQueue.back()->iStartIndex = CommandInfo.iStartIndex;
		static_cast<COMMAND_INFO_FORMATION*>(m_CommandInfoQueue.back())->iFormationIndex = static_cast<COMMAND_INFO_FORMATION*>(&CommandInfo)->iFormationIndex;
	}
}

HRESULT CNavigation::Compute_Distance_From_Goal(const _uint iGoalIndex, _fvector vCurPos, _float& fDistance)
{
	_float3			vGoalPosFloat3;
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iGoalIndex, vGoalPosFloat3)))
		return E_FAIL;

	_vector			vDirection = { XMVectorSetW(XMLoadFloat3(&vGoalPosFloat3) - vCurPos, 0.f) };
	fDistance = XMVectorGetX(XMVector3Length(vDirection));

	return S_OK;
}

_bool CNavigation::Is_Patrol() const
{
	if (nullptr == m_pAgent)
		return false;

	return m_pAgent->Is_Patrol();
}

_bool CNavigation::Is_Move() const
{
	return m_isMove;
}

_float CNavigation::Get_Speed() const
{
	if (nullptr == m_pAgent)
		return 0.f;

	return m_pAgent->Get_Speed();
}

_float CNavigation::Get_Radius() const
{
	if (nullptr == m_pAgent)
		return 0.f;

	return m_pAgent->Get_Radius();
}

_float CNavigation::Get_Height() const
{
	if (nullptr == m_pAgent)
		return 0.f;

	return m_pAgent->Get_Height();
}

_float CNavigation::Get_MaxWaitTime() const
{
	if (nullptr == m_pAgent)
		return 0.f;

	return m_pAgent->Get_MaxWaitTime();
}

_float CNavigation::Get_ObstacleRecognizeDist() const
{
	if (nullptr == m_pAgent)
		return 0.f;

	return m_pAgent->Get_ObstacleRecognizeDist();
}

void CNavigation::Set_Patrol(const _bool isPatrol)
{
	if (nullptr == m_pAgent)
		return;

	return m_pAgent->Set_Patrol(isPatrol);
}

void CNavigation::Set_Speed(const _float fSpeed)
{
	if (nullptr == m_pAgent)
		return;

	return m_pAgent->Set_Speed(fSpeed);
}

void CNavigation::Set_Radius(const _float fRadius)
{
	if (nullptr == m_pAgent)
		return;

	return m_pAgent->Set_Radius(fRadius);
}

void CNavigation::Set_Height(const _float fHeight)
{
	if (nullptr == m_pAgent)
		return;

	return m_pAgent->Set_Height(fHeight);
}

void CNavigation::Set_MaxWaitTime(const _float fMaxWaitTime)
{
	if (nullptr == m_pAgent)
		return;

	return m_pAgent->Set_MaxWaitTime(fMaxWaitTime);
}

void CNavigation::Set_ObstacleRecognizeDist(const _float fObstacleRecognizeDist)
{
	if (nullptr == m_pAgent)
		return;

	return m_pAgent->Set_ObstacleRecognizeDist(fObstacleRecognizeDist);
}

HRESULT CNavigation::Add_FormationGroup(const _uint iGroupIndexTag)
{
	if (nullptr == m_pFormation)
		return E_FAIL;

	return m_pFormation->Add_FormationGroup(iGroupIndexTag);
}

HRESULT CNavigation::Erase_FormationGroup(const _uint iGroupIndexTag)
{
	if (nullptr == m_pFormation)
		return E_FAIL;

	return m_pFormation->Erase_FormationGroup(iGroupIndexTag);
}

HRESULT CNavigation::SetUp_Formation_Path()
{
	if (nullptr == m_pFormation)
		return E_FAIL;

	auto		BestPathIndices = m_pFormation->Get_BestPathIndices();
	if (true == BestPathIndices.empty())
		return E_FAIL;

	decltype(BestPathIndices)		ResultPathIndices = {};

	_uint		iStartIndex;


	if (true == m_CommandInfoQueue.empty())
	{
		if (FAILED(Get_CurPos_Index(iStartIndex)))
			return E_FAIL;		
	}
	else
	{
		iStartIndex = m_CommandInfoQueue.back()->iGoalIndex;
	}

	list<_uint>			TempPathIndices;
	NODE_INFOS			NodeInfos;
	vector<_uint>		GoalIndices;
	for (const auto iPathIndex : BestPathIndices)
		GoalIndices.push_back(iPathIndex);

	if (FAILED(m_pGameInstance->Make_Path_Voxel(TempPathIndices, NodeInfos, iStartIndex, GoalIndices, false, false, {})))
		return E_FAIL;

	auto		iter = find(BestPathIndices.begin(), BestPathIndices.end(), TempPathIndices.back());
	while (BestPathIndices.end() != iter)
	{
		ResultPathIndices.push_back(*iter);
		++iter;
	}

	COMMAND_INFO_SAPF*			pCommandInfoPrePath = { new COMMAND_INFO_SAPF() };
	pCommandInfoPrePath->BestPathIndices = move(TempPathIndices);
	pCommandInfoPrePath->ePathMode = PATH_MODE::_SINGLE_AGENT;
	pCommandInfoPrePath->iStartIndex = pCommandInfoPrePath->BestPathIndices.front();
	pCommandInfoPrePath->iGoalIndex = pCommandInfoPrePath->BestPathIndices.back();

	m_CommandInfoQueue.push_back(pCommandInfoPrePath);

	COMMAND_INFO_SAPF*			pCommandInfoNewPath = { new COMMAND_INFO_SAPF() };
	pCommandInfoNewPath->BestPathIndices = move(ResultPathIndices);
	pCommandInfoNewPath->ePathMode = PATH_MODE::_SINGLE_AGENT;
	pCommandInfoNewPath->iStartIndex = pCommandInfoNewPath->BestPathIndices.front();
	pCommandInfoNewPath->iGoalIndex = pCommandInfoNewPath->BestPathIndices.back();

	m_CommandInfoQueue.push_back(pCommandInfoNewPath);

	return S_OK;
}

HRESULT CNavigation::SetUp_Formation_PathTest(const _uint iFormationIndexTag)
{
	if (nullptr == m_pFormation)
		return E_FAIL;

	auto		BestPathIndices = m_pFormation->Get_BestPathIndices();
	if (true == BestPathIndices.empty())
		return E_FAIL;

	COMMAND_INFO_FORMATION		CommandInfoNewPath;
	CommandInfoNewPath.BestPathIndices = move(BestPathIndices);
	CommandInfoNewPath.ePathMode = PATH_MODE::_FORMATION;
	CommandInfoNewPath.iStartIndex = CommandInfoNewPath.BestPathIndices.front();
	CommandInfoNewPath.iGoalIndex = CommandInfoNewPath.BestPathIndices.back();
	CommandInfoNewPath.iFormationIndex = iFormationIndexTag;

	Add_CommandInfo(CommandInfoNewPath);

	return S_OK;
}

void CNavigation::Recover_Wall(_fvector vStartPos, _fvector vDirection, const _float fRadius, _float3& vRecoveredDirectionFloat3)
{
	_float3				vStartPosFloat3;
	XMStoreFloat3(&vStartPosFloat3, vStartPos);

	_vector				vTargetPos = { vStartPos + XMVectorSetW(vDirection, 0.f) };
	_float3				vTargetPosFloat3;
	XMStoreFloat3(&vTargetPosFloat3, vTargetPos);

	_vector				vRecoveredDirection = { vDirection };


	_uint				iTargetVoxelIndex;
	if (FAILED(m_pGameInstance->Get_Index_Voxel(vTargetPosFloat3, iTargetVoxelIndex)))
		return;
	vector<_uint>		NeighborIndices = { move(m_pGameInstance->Get_NeighborIndices_All_Voxel(iTargetVoxelIndex, VOXEL_LAYER::_STATIC)) };

	/*for (auto iter = NeighborIndices.begin(); iter != NeighborIndices.end(); )
	{
		const _uint			iVoxelIndex = { *iter };yy
		VOXEL_ID			eVoxelID = { VOXEL_ID::_END };
		m_pGameInstance->Get_VoxelID(iVoxelIndex, eVoxelID, VOXEL_LAYER::_STATIC);

		if (VOXEL_ID::_WALL != eVoxelID)
			iter = NeighborIndices.erase(iter);
		else
			++iter;
	}*/

	sort(NeighborIndices.begin(), NeighborIndices.end(), [&](const _uint iSrc, const _uint iDst) {
		_float3			vSrcPos, vDstPos;
		m_pGameInstance->Get_WorldPosition_Voxel(iSrc, vSrcPos);
		m_pGameInstance->Get_WorldPosition_Voxel(iDst, vDstPos);

		_float3			vNearestPosSrc, vNearestPosDst;
		m_pGameInstance->Compute_Nearest_Position_VoxelBounding(vStartPosFloat3, vSrcPos, vNearestPosSrc);
		m_pGameInstance->Compute_Nearest_Position_VoxelBounding(vStartPosFloat3, vDstPos, vNearestPosDst);

		_float			fDistSrc = { XMVectorGetX(XMVector3Length(vStartPos - XMLoadFloat3(&vNearestPosSrc))) };
		_float			fDistDst = { XMVectorGetX(XMVector3Length(vStartPos - XMLoadFloat3(&vNearestPosDst))) };

		return fDistSrc < fDistDst;
		});

	for (const auto iNeighborIndex : NeighborIndices)
	{
		VOXEL_ID			eVoxelID;
		if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eVoxelID, VOXEL_LAYER::_STATIC)) ||
			VOXEL_ID::_WALL != eVoxelID)
			continue;

		_float3				vNeighborPosFloat3;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNeighborIndex, vNeighborPosFloat3)))
			return;

		_float3				vNearestPosNeighborFloat3;
		if (FAILED(m_pGameInstance->Compute_Nearest_Position_VoxelBounding(vTargetPosFloat3, vNeighborPosFloat3, vNearestPosNeighborFloat3)))
			return;

		_vector				vNearestPosNeighbor = { XMLoadFloat3(&vNearestPosNeighborFloat3) };
		const _float		fDistNearest = { XMVectorGetX(XMVector3Length(vNearestPosNeighbor - vTargetPos)) };
		const _float		fRecoverLen = { 0.25f - fDistNearest };

		if (0.f >= fRecoverLen)
			continue;

		vTargetPos = vTargetPos + XMVector3Normalize(vTargetPos - vNearestPosNeighbor) * fRecoverLen;
		vRecoveredDirection = vTargetPos - vStartPos;
		XMStoreFloat3(&vTargetPosFloat3, vTargetPos);
	}


	XMStoreFloat3(&vRecoveredDirectionFloat3, vRecoveredDirection);
}

void CNavigation::Next_Move()
{
	if (nullptr == m_pAgent)
		return;

	if (PATH_MODE::_MULTI_AGENT == m_CommandInfoQueue.front()->ePathMode)
	{
		const _uint			iFlowIndex = { static_cast<COMMAND_INFO_MAPF*>(m_CommandInfoQueue.front())->iFlowIndex };
		m_pGameInstance->Release_FlowField(iFlowIndex);
	}

	else if (PATH_MODE::_FORMATION == m_CommandInfoQueue.front()->ePathMode)
	{
		const _uint			iFormationIndex = { static_cast<COMMAND_INFO_FORMATION*>(m_CommandInfoQueue.front())->iFormationIndex };
		m_pGameInstance->Erase_Formation(m_pFormation, iFormationIndex);
	}


	Safe_Delete(*(m_CommandInfoQueue.begin()));
	m_CommandInfoQueue.erase(m_CommandInfoQueue.begin());

	if (true == m_CommandInfoQueue.empty())
	{
		m_pAgent->Stop_Move();
		m_isMove = false;
	}
	else
	{
		m_pAgent->Start_Move();
	}
}

HRESULT CNavigation::Ready_Detour_Static_Obstacle(const list<_uint>& OriginPath, list<_uint>& NewPath, const _uint iMoveTargetIndex, _uint& iNewStartIndex, vector<_uint>& NewGoals, const vector<_uint>& MyReserveIndices)
{
	auto		iter{ OriginPath.begin() };
	advance(iter, iMoveTargetIndex);

	iNewStartIndex = (*iter);

	while (true)
	{
		if (true == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_OBSTACLE_STATIC) ||
			false == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_STATIC))
		{
			if (iter == OriginPath.begin())
				return E_FAIL;
			else
				iNewStartIndex = { *(--iter) };
		}
		else
			break;
	}

	_bool			isOldPath = { true };
	for (_uint i = 0; i < OriginPath.size(); ++i)
	{
		iter = OriginPath.begin();
		advance(iter, i);

		if (iNewStartIndex == *iter)
			isOldPath = false;

		if (true == isOldPath)		//	지나온 경로
			NewPath.push_back(*iter);
		else						//	나아갈 경로
			NewGoals.push_back(*iter);
	}

	//	장애물 (Obstacle에 속하는 경로들을 기존 경로에서 쳐낸다.)
	for (auto iter = NewGoals.begin(); iter != NewGoals.end(); )
	{
		const _uint		iIndex = { *iter };

		if (true == m_pGameInstance->Is_Exist_Voxel(iIndex, VOXEL_LAYER::_OBSTACLE_STATIC) ||
			false == m_pGameInstance->Is_Exist_Voxel(iIndex, VOXEL_LAYER::_STATIC))
			iter = NewGoals.erase(iter);
		else
			++iter;
	}

	if (true == NewGoals.empty())
		return E_FAIL;

	//	부모 노드로 이어질때 골노드로 이어지지못하는 중간에 잘린 경로를 쳐낸다.
	auto		lastIterGoals = NewGoals.end();
	--lastIterGoals;
	auto		lastIterOriginPath = OriginPath.end();
	--lastIterOriginPath;

	_uint		iLastIndexGoals = { *lastIterGoals };
	_uint		iLastIndexOriginPath = { *lastIterOriginPath };

	vector<_uint>		TempGoals;

	while (NewGoals.begin() != lastIterGoals &&
		iLastIndexGoals == iLastIndexOriginPath)
	{
		TempGoals.push_back(iLastIndexGoals);

		--lastIterGoals;
		--lastIterOriginPath;

		iLastIndexGoals = *lastIterGoals;
		iLastIndexOriginPath = *lastIterOriginPath;
	}

	NewGoals = move(TempGoals);
	reverse(NewGoals.begin(), NewGoals.end());

	return S_OK;
}

HRESULT CNavigation::Ready_Detour_Dynamic_Obstacle(const list<_uint>& OriginPath, list<_uint>& NewPath, const _uint iMoveTargetIndex, _uint& iNewStartIndex, vector<_uint>& NewGoals, const vector<_uint>& MyReserveIndices)
{
	auto		iter{ OriginPath.begin() };
	advance(iter, iMoveTargetIndex);

	iNewStartIndex = { *iter };

	while (true)
	{
		if (true == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_OBSTACLE_STATIC) ||
			false == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_STATIC) ||
			(true == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC) &&
				MyReserveIndices.end() == find(MyReserveIndices.begin(), MyReserveIndices.end(), iNewStartIndex)))
		{
			if (iter == OriginPath.begin())
				return E_FAIL;
			else
				iNewStartIndex = { *(--iter) };
		}
		else
			break;
	}

	_bool			isOldPath = { true };
	for (_uint i = 0; i < OriginPath.size(); ++i)
	{
		iter = OriginPath.begin();
		advance(iter, i);

		if (iNewStartIndex == *iter)
			isOldPath = false;

		if (true == isOldPath)
			NewPath.push_back(*iter);
		else
			NewGoals.push_back(*iter);
	}

	//	장애물 (Obstacle에 속하는 경로들을 기존 경로에서 쳐낸다.)
	_bool			isEraseOnceMore = { false };
	for (auto iter = NewGoals.begin(); iter != NewGoals.end(); )
	{
		const _uint		iIndex = { *iter };

		if (true == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_OBSTACLE_STATIC) ||
			false == m_pGameInstance->Is_Exist_Voxel(iNewStartIndex, VOXEL_LAYER::_STATIC) ||
			(true == m_pGameInstance->Is_Exist_Voxel(iIndex, VOXEL_LAYER::_OBSTACLE_DYNAMIC) &&
			MyReserveIndices.end() == find(MyReserveIndices.begin(), MyReserveIndices.end(), iIndex)) ||
			false == m_pGameInstance->Is_Exist_Voxel(iIndex, VOXEL_LAYER::_STATIC))
		{
			iter = NewGoals.erase(iter);
			isEraseOnceMore = true;
		}
		else
		{
			++iter;
			if (true == isEraseOnceMore)
				break;
		}
	}

	if (true == NewGoals.empty())
		return E_FAIL;

	auto		lastIterGoals = NewGoals.end();
	--lastIterGoals;
	auto		lastIterOriginPath = OriginPath.end();
	--lastIterOriginPath;

	_uint		iLastIndexGoals = { *lastIterGoals };
	_uint		iLastIndexOriginPath = { *lastIterOriginPath };

	vector<_uint>		TempGoals;

	while (NewGoals.begin() != lastIterGoals &&
		iLastIndexGoals == iLastIndexOriginPath)
	{
		TempGoals.push_back(iLastIndexGoals);

		--lastIterGoals;
		--lastIterOriginPath;

		iLastIndexGoals = *lastIterGoals;
		iLastIndexOriginPath = *lastIterOriginPath;
	}

	NewGoals = move(TempGoals);
	reverse(NewGoals.begin(), NewGoals.end());

	return S_OK;
}

HRESULT CNavigation::End_Detour()
{
	//		머지

	//	이전 지나온 경로

	//	새로운 경로

	return S_OK;
}

void CNavigation::Set_ReserveSensortive(const _float fSensortive)
{
	return CAgent::Set_ReserveSensortive(fSensortive);
}

_float CNavigation::Get_ReserveSensortive()
{
	return CAgent::Get_ReserveSensortive();
}

CNavigation* CNavigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNavigation* pInstance = new CNavigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CNavigation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CNavigation::Clone(void* pArg)
{
	CNavigation* pInstance = { new CNavigation(*this) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CNavigation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNavigation::Free()
{
	__super::Free();

	m_pGameInstance->Erase_Boid(m_pBoid);

	Clear_CommandQueue();

	Safe_Release(m_pAgent);
	Safe_Release(m_pBoid);
	Safe_Release(m_pFormation);
	Safe_Release(m_pTransformCom);
}
