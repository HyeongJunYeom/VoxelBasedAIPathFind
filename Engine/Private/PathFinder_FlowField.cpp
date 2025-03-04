#include "PathFinder_FlowField.h"
#include "GameInstance.h"


CPathFinder_FlowField::CPathFinder_FlowField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPathFinder_FlowField::Initialize()
{
	m_FlowFieldInfos.resize(ms_iNumFlowFieldInfos);

	for (auto& pFlowFieldInfo : m_FlowFieldInfos)
		pFlowFieldInfo = new FLOW_FIELD_INFO{};

	return S_OK;
}

HRESULT CPathFinder_FlowField::Make_VectorFiled_Daijkstra(const vector<_uint>& GoalIndices, const _uint iNumStartAgent, _uint& iIndexTag)
{
	if (m_iActiveCnt >= ms_iNumFlowFieldInfos)
		return E_FAIL;

	unordered_map<_uint, _float>			DistanceMap;
	unordered_map<_uint, _float3>			FlowField;
	unordered_map<_uint, _uint>				ParentMap;

	if (FAILED(Make_DistanceMap_Daijkstra(GoalIndices, DistanceMap)) ||
		FAILED(Make_FlowFieldMap(DistanceMap, FlowField)) ||
		FAILED(Make_ParentMap(DistanceMap, ParentMap)))
		return E_FAIL;

	FLOW_FIELD_INFO			FlowFieldInfo;
	FlowFieldInfo.DistanceMap = move(DistanceMap);
	FlowFieldInfo.FlowField = move(FlowField);
	FlowFieldInfo.ParentMap = move(ParentMap);

	if (FAILED(Add_FlowField(FlowFieldInfo, iNumStartAgent, iIndexTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPathFinder_FlowField::Make_VectorFiled_A_Star(const vector<_uint>& StartIndices, const _uint iGoalIndex, const NODE_INFOS& NodeInfos, const _uint iNumStartAgent, _uint& iIndexTag)
{
	if (m_iActiveCnt >= ms_iNumFlowFieldInfos ||
		true == NodeInfos.empty())
		return E_FAIL;

	unordered_map<_uint, _float3>			FlowField;
	unordered_map<_uint, _uint>				ParentMap;
	unordered_map<_uint, _float>			DistanceMap;
	for (auto Pair : NodeInfos)
	{
		const _uint		iVoxelIndex = { Pair.first };
		const _float	fDist = { Pair.second.fG_Cost };
		const _uint		iParentIndex = { Pair.second.iParentIndex };

		DistanceMap[iVoxelIndex] = fDist;
		ParentMap[iVoxelIndex] = iParentIndex;
	}


	if (FAILED(Make_FlowFieldMap(DistanceMap, FlowField)))
		return E_FAIL;

	FLOW_FIELD_INFO			FlowFieldInfo;
	FlowFieldInfo.DistanceMap = move(DistanceMap);
	FlowFieldInfo.FlowField = move(FlowField);
	FlowFieldInfo.ParentMap = move(ParentMap);

	if (FAILED(Add_FlowField(FlowFieldInfo, iNumStartAgent, iIndexTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPathFinder_FlowField::Get_Direction_FlowField(const _float3& vCurPos, const _uint iIndexTag, _float3& vDirection)
{
	if (false == Is_Active_Field(iIndexTag))
		return E_FAIL;

	_uint3			vIndexPos;
	m_pGameInstance->Get_IndexPosition_Voxel(vCurPos, vIndexPos);

	_uint			iVoxelIndex;
	if (FAILED(m_pGameInstance->Get_Index_Voxel(vIndexPos, iVoxelIndex)))
		return E_FAIL;

	unordered_map<_uint, _float3>&			FlowField = { m_FlowFieldInfos[iIndexTag]->FlowField };

	auto iter{ FlowField.find(iVoxelIndex) };
	if (iter == FlowField.end())
		return E_FAIL;

	vDirection = iter->second;

	return S_OK;
}

HRESULT CPathFinder_FlowField::Get_ParentIndex_FlowField(const _float3& vCurPos, const _uint iIndexTag, _uint& iParentIndex)
{
	if (false == Is_Active_Field(iIndexTag))
		return E_FAIL;

	_uint3			vIndexPos;
	m_pGameInstance->Get_IndexPosition_Voxel(vCurPos, vIndexPos);

	_uint			iVoxelIndex;
	if (FAILED(m_pGameInstance->Get_Index_Voxel(vIndexPos, iVoxelIndex)))
		return E_FAIL;

	return Get_ParentIndex_FlowField(iVoxelIndex, iIndexTag, iParentIndex);
}

HRESULT CPathFinder_FlowField::Get_ParentIndex_FlowField(const _uint iVoxelIndex, const _uint iIndexTag, _uint& iParentIndex)
{
	if (false == Is_Active_Field(iIndexTag))
		return E_FAIL;

	unordered_map<_uint, _uint>& ParentMap = { m_FlowFieldInfos[iIndexTag]->ParentMap };

	auto iter{ ParentMap.find(iVoxelIndex) };
	if (iter == ParentMap.end())
		return E_FAIL;

	iParentIndex = iter->second;
	return S_OK;
}

HRESULT CPathFinder_FlowField::Add_Distances(const _uint iIndexTag, const unordered_map<_uint, _float>& AdditionalDists)
{
	return E_NOTIMPL;
}

struct CMP_VECTOR_FIELD
{
	_bool operator() (const pair<_float, _uint>& Src, const pair<_float, _uint>& Dst)
	{
		return Src.first > Dst.first;
	}
};

HRESULT CPathFinder_FlowField::Make_DistanceMap_Daijkstra(const vector<_uint>& GoalIndices, unordered_map<_uint, _float>& DistanceMap)
{
	priority_queue<pair<_float, _uint>, vector<pair<_float, _uint>>, CMP_VECTOR_FIELD>		OpenQueue;

	for (const auto iGoalIndex : GoalIndices)
	{
		_float3				vGoalPos = {};
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iGoalIndex, vGoalPos)))
			continue;

		OpenQueue.push({ 0.f, iGoalIndex });
		DistanceMap[iGoalIndex] = 0.f;
	}

	while (false == OpenQueue.empty())
	{
		const _uint			iCurIndex = { OpenQueue.top().second };
		const _float		fCurDist = { OpenQueue.top().first };
		OpenQueue.pop();

		if (DistanceMap[iCurIndex] < fCurDist)
			continue;

		_float3				vCurPos;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurIndex, vCurPos)))
			return E_FAIL;

		vector<_uint>		NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC) };

		for (const _uint iNeighborIndex : NeighborIndices)
		{
			_float3				vNeighborPos = {};
			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNeighborIndex, vNeighborPos)))
				return E_FAIL;

			_byte				bOpenessScore;
			if (FAILED(m_pGameInstance->Get_VoxelOpenessScore(iNeighborIndex, bOpenessScore, VOXEL_LAYER::_STATIC)))
				return E_FAIL;

			const _float		fWeight = { max((50.f - static_cast<_float>(bOpenessScore)), 0.f) * 0.02f };

			const _float		fDistToNeighbor = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vNeighborPos) - XMLoadFloat3(&vCurPos))) };
			const _float		fNewDist = { (fDistToNeighbor + (fWeight * 100.f)) + fCurDist };
			if (DistanceMap.end() != DistanceMap.find(iNeighborIndex) &&
				fNewDist >= DistanceMap[iNeighborIndex])
				continue;

			VOXEL_ID			eVoxelID;
			if (FAILED(m_pGameInstance->Get_VoxelID(iNeighborIndex, eVoxelID, VOXEL_LAYER::_STATIC)))
				return E_FAIL;

			if (VOXEL_ID::_FLOOR == eVoxelID)
			{
				OpenQueue.push({ fNewDist, iNeighborIndex });
				DistanceMap[iNeighborIndex] = fNewDist;
			}
		}
	}

	return S_OK;
}

HRESULT CPathFinder_FlowField::Make_FlowFieldMap(const unordered_map<_uint, _float>& DistanceMap, unordered_map<_uint, _float3>& FlowFiledMap)
{
	for (auto Pair : DistanceMap)
	{
		const _uint			iCurIndex = { Pair.first };
		const _float		fCurDist = { DistanceMap.find(iCurIndex)->second };

		if (EPSILON >= fCurDist)
			continue;

		_float3				vCurPosFloat3;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurIndex, vCurPosFloat3)))
			return E_FAIL;
		_vector				vCurPos = { XMLoadFloat3(&vCurPosFloat3) };

		vector<_uint>		NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC) };

#pragma region 유연한 벡터
		//_vector				vAccDirection = { XMVectorZero() };

		//for (auto iNeighborIndex : NeighborIndices)
		//{
		//	auto iter{ DistanceMap.find(iNeighborIndex) };
		//	//	미방문 || 벽
		//	if (DistanceMap.end() == iter)
		//		continue;

		//	_float3			vNeighborPosFloat3;
		//	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNeighborIndex, vNeighborPosFloat3)))
		//		return E_FAIL;
		//	_vector			vNeighborPos = { XMLoadFloat3(&vNeighborPosFloat3) };
		//	_vector			vCurDirection = { vCurPos - vNeighborPos };

		//	const _float	fNeighborDist = { DistanceMap.find(iNeighborIndex)->second };
		//	vAccDirection += XMVector3Normalize(vCurDirection) * (fNeighborDist - fCurDist);
		//}

		//if(XMVectorGetX(XMVector3Length(vAccDirection)) <= EPSILON)
		//	continue;;

		//_float3			vResultDirectionFloat3;
		//_vector			vResultDirection = { XMVector3Normalize(vAccDirection) };

		//XMStoreFloat3(&vResultDirectionFloat3, vResultDirection);

		//FlowFiledMap[iCurIndex] = vResultDirectionFloat3;
#pragma endregion
#pragma region 45도 단위 벡터
		_float		fMinDist = { FLT_MAX };
		_int		iResultIndex  = { -1 };

		for (auto iNeighborIndex : NeighborIndices)
		{
			auto iter{ DistanceMap.find(iNeighborIndex) };
			//	미방문 || 벽
			if (DistanceMap.end() == iter)
				continue;

			if (fMinDist <= iter->second)
				continue;

			fMinDist = iter->second;
			iResultIndex = static_cast<_int>(iNeighborIndex);
		}

		if (-1 != iResultIndex)
		{
			_float3			vResultPos;
			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iResultIndex, vResultPos)))
				return E_FAIL;

			_vector			vResultDirection = { XMVector3Normalize(XMLoadFloat3(&vResultPos) - vCurPos) };
			_float3			vResultDirectionFloat3;

			XMStoreFloat3(&vResultDirectionFloat3, vResultDirection);

			FlowFiledMap[iCurIndex] = vResultDirectionFloat3;
		}
#pragma endregion
	}

	return S_OK;
}

HRESULT CPathFinder_FlowField::Make_ParentMap(const unordered_map<_uint, _float>& DistanceMap, unordered_map<_uint, _uint>& ParentMap)
{
	for (auto Pair : DistanceMap)
	{
		const _uint			iCurIndex = { Pair.first };
		const _float		fCurDist = { DistanceMap.find(iCurIndex)->second };

		if (EPSILON >= fCurDist)
			continue;

		_float3				vCurPosFloat3;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurIndex, vCurPosFloat3)))
			return E_FAIL;
		_vector				vCurPos = { XMLoadFloat3(&vCurPosFloat3) };

		vector<_uint>		NeighborIndices = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurIndex, VOXEL_LAYER::_STATIC) };

		_float		fMinDist = { FLT_MAX };
		_int		iResultIndex = { -1 };

		for (auto iNeighborIndex : NeighborIndices)
		{
			auto iter{ DistanceMap.find(iNeighborIndex) };
			//	미방문 || 벽
			if (DistanceMap.end() == iter)
				continue;

			if (fMinDist <= iter->second)
				continue;

			fMinDist = iter->second;
			iResultIndex = static_cast<_int>(iNeighborIndex);
		}

		if (-1 != iResultIndex)
			ParentMap[iCurIndex] = iResultIndex;
	}

	return S_OK;
}

_float CPathFinder_FlowField::Get_DistanceFromGoal(const _uint iVoxelIndex, const _uint iIndexTag)
{
	if (false == Is_Active_Field(iIndexTag))
		return FLT_MAX;

	unordered_map<_uint, _float>&		DistanceMap = { m_FlowFieldInfos[iIndexTag]->DistanceMap };

	auto iter{ DistanceMap.find(iVoxelIndex) };
	if (iter == DistanceMap.end())
		return FLT_MAX;

	return DistanceMap[iVoxelIndex];
}

const unordered_map<_uint, _float3>& CPathFinder_FlowField::Get_FlowField(const _uint iIndexTag)
{
	static unordered_map<_uint, _float3>		Temp;
	if (false == Is_Active_Field(iIndexTag))
		return Temp;

	return m_FlowFieldInfos[iIndexTag]->FlowField;
}

HRESULT CPathFinder_FlowField::AddRef_FlowField(const _uint iIndexTag)
{
	if (ms_iNumFlowFieldInfos <= iIndexTag)
		return E_FAIL;

	++m_FlowFieldInfos[iIndexTag]->iRefCnt;
	return S_OK;
}

HRESULT CPathFinder_FlowField::Release_FlowField(const _uint iIndexTag)
{
	if (ms_iNumFlowFieldInfos <= iIndexTag ||
		0 >= m_FlowFieldInfos[iIndexTag]->iRefCnt)
		return E_FAIL;

	--m_FlowFieldInfos[iIndexTag]->iRefCnt;
	if (0 >= m_FlowFieldInfos[iIndexTag]->iRefCnt)
	{
		m_FlowFieldInfos[iIndexTag]->DistanceMap.clear();
		m_FlowFieldInfos[iIndexTag]->FlowField.clear();
		m_FlowFieldInfos[iIndexTag]->ParentMap.clear();
		--m_iActiveCnt;
	}

	return S_OK;
}

vector<_uint> CPathFinder_FlowField::Get_Active_FlowField_RefCnt()
{
	vector<_uint>			RefCnts;
	for (_uint i = 0; i < ms_iNumFlowFieldInfos; ++i)
	{
		if (0 < m_FlowFieldInfos[i]->iRefCnt)
			RefCnts.push_back(m_FlowFieldInfos[i]->iRefCnt);
	}

	return RefCnts;
}

vector<_uint> CPathFinder_FlowField::Get_Active_FlowField_Indices()
{
	vector<_uint>			ActiveIndices;
	for (_uint i = 0; i < ms_iNumFlowFieldInfos; ++i)
	{
		if (0 < m_FlowFieldInfos[i]->iRefCnt)
			ActiveIndices.push_back(i);
	}

	return ActiveIndices;
}

_bool CPathFinder_FlowField::Is_Active_Field(const _uint iIndexTag)
{
	if (ms_iNumFlowFieldInfos <= iIndexTag)
		return false;

	return 0 < m_FlowFieldInfos[iIndexTag]->iRefCnt;
}

HRESULT CPathFinder_FlowField::Find_Non_Active_IndexTag(_uint& iIndexTag)
{
	for (_uint i = 0; i < ms_iNumFlowFieldInfos; ++i)
	{
		if (false == Is_Active_Field(i))
		{
			iIndexTag = i;
			return S_OK;
		}
	}

	return E_FAIL;
}

HRESULT CPathFinder_FlowField::Add_FlowField(FLOW_FIELD_INFO& FlowFieldInfo, const _uint iStartRefCnt, _uint& iIndexTag)
{
	if(FAILED(Find_Non_Active_IndexTag(iIndexTag)))
		return E_FAIL;

	m_FlowFieldInfos[iIndexTag]->iRefCnt = iStartRefCnt;
	m_FlowFieldInfos[iIndexTag]->DistanceMap = move(FlowFieldInfo.DistanceMap);
	m_FlowFieldInfos[iIndexTag]->FlowField = move(FlowFieldInfo.FlowField);
	m_FlowFieldInfos[iIndexTag]->ParentMap = move(FlowFieldInfo.ParentMap);

	++m_iActiveCnt;

	return S_OK;
}

CPathFinder_FlowField* CPathFinder_FlowField::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPathFinder_FlowField*		pInstance = { new CPathFinder_FlowField(pDevice, pContext) };

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CPathFinder_FlowField"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPathFinder_FlowField::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (auto pFlowFieldInfo : m_FlowFieldInfos)
	{
		Safe_Delete(pFlowFieldInfo);
		pFlowFieldInfo = nullptr;
	}

	m_FlowFieldInfos.clear();
}
