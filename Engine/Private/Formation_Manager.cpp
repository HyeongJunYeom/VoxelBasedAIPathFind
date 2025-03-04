#include "Formation_Manager.h"
#include "GameInstance.h"
#include "Formation.h"

CFormation_Manager::CFormation_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CFormation_Manager::Initialize()
{
	m_GroupInfos.resize(ms_iNumFormationGroupInfos);
	for (auto& pGroupInfo : m_GroupInfos)
		pGroupInfo = new FORMATION_GROUP_INFO();

	return S_OK;
}

void CFormation_Manager::Tick(_float fTimeDelta)
{
	//for (_uint iPathIndex = 0; iPathIndex < m_Groups.size(); ++iPathIndex)
	//	if (FAILED(Initiate_Formation(iPathIndex)))
	//		return;

	if (m_pGameInstance->Get_KeyState('F') == DOWN)
	{
		m_isFourLine = !m_isFourLine;
	}

	//	m_pGameInstance->Add_Render_Path()

	for (auto pInfo : m_GroupInfos)
	{
		if (false == pInfo->isActive)
			continue;

		if (FAILED(m_pGameInstance->Add_Render_Path(pInfo->VirtualPath)))
			continue;

		if (FAILED(Update_Formations(fTimeDelta, pInfo)))
			continue;
	}
}

HRESULT CFormation_Manager::Add_Group(_uint& iGroupIndex)
{
	for (_uint i = 0; i < ms_iNumFormationGroupInfos; ++i)
	{
		if (false == m_GroupInfos[i]->isActive)
		{
			m_GroupInfos[i]->isActive = true;
			iGroupIndex = i;
			return S_OK;
		}
	}

	return E_FAIL;
}

HRESULT CFormation_Manager::Erase_Group(const _uint iGroupIndex)
{
	if (iGroupIndex >= ms_iNumFormationGroupInfos)
		return E_FAIL;

	m_GroupInfos[iGroupIndex]->ColsPathes.clear();
	m_GroupInfos[iGroupIndex]->Formations.clear();
	m_GroupInfos[iGroupIndex]->VirtualPath.clear();

	m_GroupInfos[iGroupIndex]->isActive = false;

	return S_OK;
}

HRESULT CFormation_Manager::Add_Formation(CFormation* pFormation, const _uint iGroupIndex)
{
	if (nullptr == pFormation ||
		iGroupIndex >= ms_iNumFormationGroupInfos ||
		false == m_GroupInfos[iGroupIndex]->isActive)
		return E_FAIL;

	auto&		Formations = m_GroupInfos[iGroupIndex]->Formations;
	auto		iter = find(Formations.begin(), Formations.end(), pFormation);
	if (iter != Formations.end())
	{
		MSG_BOX(TEXT("Already Included Formation"));
		return E_FAIL;
	}

	Formations.push_back(pFormation);
	Safe_AddRef(pFormation);
	return S_OK;
}

HRESULT CFormation_Manager::Erase_Formation(CFormation* pFormation, const _uint iGroupIndex)
{
	if (nullptr == pFormation ||
		iGroupIndex >= ms_iNumFormationGroupInfos ||
		false == m_GroupInfos[iGroupIndex]->isActive)
		return E_FAIL;

	auto&		Formations = m_GroupInfos[iGroupIndex]->Formations;
	auto		iter = find(Formations.begin(), Formations.end(), pFormation);
	if (iter == Formations.end())
	{
		MSG_BOX(TEXT("Not Included Formation"));
		return E_FAIL;
	}

	pFormation->Set_MaxWaitTime(0.25f);
	pFormation->Set_Speed(3.f);
	Formations.erase(iter);
	Safe_Release(pFormation);

	if (true == Formations.empty())
		Erase_Group(iGroupIndex);

	return S_OK;
}

HRESULT CFormation_Manager::Update_Formations(_float fTimeDelta, FORMATION_GROUP_INFO* pGroupInfo)
{
	_float		fRatio = pGroupInfo->fRatio;
	const _float		fVirtualPathTotalLength = { pGroupInfo->fVirtualPathTotalLength };
	const _float		fMovedLength = { fVirtualPathTotalLength * fRatio };
	const _uint			iNumAgent = static_cast<_uint>(pGroupInfo->Formations.size());

	_float			fRatioSum{};
	for (_uint i = 0; i < iNumAgent; ++i)
	{
		auto pFormation = pGroupInfo->Formations[i];
		_float			fCurAgentRatio = pFormation->Get_LastRatio();
		fCurAgentRatio -= pGroupInfo->RatioOffsets[i];
		fRatioSum += fCurAgentRatio;
	}

	fRatio = { fRatioSum / iNumAgent };
	pGroupInfo->fRatio = fRatio;

	const _float		fMinRatioRange = { pGroupInfo->fMinRange / pGroupInfo->fVirtualPathTotalLength };
	const _float		fMaxRatioRange = { pGroupInfo->fMaxRange / pGroupInfo->fVirtualPathTotalLength };

	_float				fMinRatio = { fmaxf(fRatio + fMinRatioRange, 0.f) };
	_float				fMaxRatio = { fminf(fRatio + fMaxRatioRange, 1.f) };

	_uint				iNumFallBehind = {};

	for (_uint iFormationIndex = 0; iFormationIndex < pGroupInfo->Formations.size(); ++iFormationIndex)
	{
		CFormation*			pFormation = { pGroupInfo->Formations[iFormationIndex] };
		_float				fRatioOffset = { pGroupInfo->RatioOffsets[iFormationIndex] };

		if (FAILED(pFormation->Update_CurrentRatio()))
			continue;

		if (FAILED(pFormation->Update_Speed(fRatio, fMinRatio + fRatioOffset, fMaxRatio + fRatioOffset)))
			continue;
	}

	if (true == pGroupInfo->Formations.empty())
	{
		pGroupInfo->isActive = false;
	}

	const _float				fRatioAgentFall = { static_cast<_float>(iNumFallBehind) / iNumAgent };

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex)
{
	if (FAILED(Initiate_Default_Param(iGroupIndexTag)) ||
		//	FAILED(Initiate_Formation_Virtual_Path(iGroupIndexTag, iStartIndex, iGoalIndex)) ||
		FAILED(Initiate_Formation_Virtual_Path_FlowField(iGroupIndexTag, iStartIndex, iGoalIndex)) ||
		FAILED(Initiate_Formation_Pathes(iGroupIndexTag)) ||
		FAILED(Initiate_Formation_Rollback_Virtual_Path(iGroupIndexTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Default_Param(const _uint iGroupIndexTag)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;

	if (m_eFormationType >= FORMATION_TYPE::_END)
		return E_FAIL;

	m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag = UINT_MAX;

	m_GroupInfos[iGroupIndexTag]->fRatio = 0.f;

	m_GroupInfos[iGroupIndexTag]->fMinSpeed = 2.f;
	m_GroupInfos[iGroupIndexTag]->fMaxSpeed = 8.f;

	m_GroupInfos[iGroupIndexTag]->fMinRange = -5.f;
	m_GroupInfos[iGroupIndexTag]->fMaxRange = 5.f;

	m_GroupInfos[iGroupIndexTag]->ColsPathes.clear();
	m_GroupInfos[iGroupIndexTag]->ColsRatios.clear();

	for (auto pFormation : m_GroupInfos[iGroupIndexTag]->Formations)
	{
		pFormation->Set_Speed(m_GroupInfos[iGroupIndexTag]->fMinSpeed);
		pFormation->Reset_LastRatio();
	}

	m_iNumAgent = { static_cast<_uint>(m_GroupInfos[iGroupIndexTag]->Formations.size()) };
	m_GroupInfos[iGroupIndexTag]->RatioOffsets.resize(m_iNumAgent);


	if (FAILED(Get_Formation_Offsets(m_Offset_Width, m_Offset_Depth, m_iNumAgent)))
		return E_FAIL;
	if (FAILED(Compute_Formation_Offsets(m_Offset_Width, m_Offset_Depth, m_iNumAgent, m_iMinOffsetWidth, m_iMaxOffsetWidth, m_iMinOffsetDepth, m_iMaxOffsetDepth)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_Pathes(const _uint iGroupIndexTag)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;

	const _uint			iNumAgent = { m_iNumAgent };

	vector<_int>		Offset_Width = m_Offset_Width;
	vector<_int>		Offset_Depth = m_Offset_Depth;

	_int				iMinOffsetWidth = { m_iMinOffsetWidth };
	_int				iMaxOffsetWidth = { m_iMaxOffsetWidth };
	_int				iMinOffsetDepth = { m_iMinOffsetDepth };
	_int				iMaxOffsetDepth = { m_iMaxOffsetDepth };

	const _uint			iRangeWidth = { static_cast<_uint>(iMaxOffsetWidth - iMinOffsetWidth) };
	const _uint			iRangeDepth = { static_cast<_uint>(iMaxOffsetDepth - iMinOffsetDepth) };
	
	const _float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };

	map<_int, pair<_int, _int>>			ColsRanges;
	map<_int, vector<_int>>				ColsRows;
	set<_int>							ColsOffsets;
	for (_uint i = 0; i < iNumAgent; ++i)
	{
		const _int		iZ = { Offset_Depth[i] };
		const _int		iX = { Offset_Width[i] };

		ColsRows[iX].push_back(iX);

		auto iter = ColsRanges.find(iX);
		if (iter == ColsRanges.end())
		{
			ColsRanges[iX] = { iZ,iZ };
			ColsOffsets.insert(iX);
		}
		else
		{
			iter->second.first = min(iter->second.first, iZ);
			iter->second.second = max(iter->second.second, iZ);
		}
	}

	//	1.
	/*		한 붓 그리기
	
	전체길이 / (현재 오프셋 - 최소 오프셋) => 열간의 간격을 만듦		
	열의 개수가 0이면 간격은 0으로 설정 => 가이드 라인 
	각 경로상의 노드별로 다음 노드를 바라보고 좌우측위치로 노드를 배치함
	좌우로만 전개할때 만약 y축상의 밟을 땅이 이어지는 범위를 체크해야함
	한걸음씩 뻗어보되 y는 1의 차이를 둬도된다. ( 이웃함 )
	*/  //  한 붓 그리기
	vector<vector<_float3>>		AquiredPosesRows;
	vector<vector<_bool>>		AquiredSucceed;
	if (FAILED(Compute_Aquired_Range(iMinOffsetWidth, iMaxOffsetWidth, m_GroupInfos[iGroupIndexTag]->VirtualPath, AquiredPosesRows, AquiredSucceed)))
		return E_FAIL;

	if (FAILED(Optimize_Aquired_Voxels(AquiredPosesRows, AquiredSucceed)))
		return E_FAIL;

	const auto&				VirtualPath = m_GroupInfos[iGroupIndexTag]->VirtualPath;
	const _uint				iNumPathIndices = { static_cast<_uint>(VirtualPath.size()) };
	list<vector<_uint>>		PathesRows;
	PathesRows.resize(iNumPathIndices);
	for (auto& RowIndices : PathesRows)
		RowIndices.resize(static_cast<size_t>(iRangeWidth) + 1);

	m_GroupInfos[iGroupIndexTag]->ColsRatios.resize((static_cast<size_t>(iRangeWidth) + 1));

	auto				iterColsPathes = PathesRows.begin();
	_float				fAccDist = {};
	_float3				vPreVoxelPosVirtualPath;
	_float3				vCurVoxelPosVirtualPath;

	m_pGameInstance->Get_WorldPosition_Voxel(VirtualPath.front(), vCurVoxelPosVirtualPath);

	for (_uint iPathIndex = 0; iPathIndex < iNumPathIndices; ++iPathIndex, ++iterColsPathes)
	{
		auto&				CurPathCols = (*iterColsPathes);

		vPreVoxelPosVirtualPath = vCurVoxelPosVirtualPath;

		auto iterPath = VirtualPath.begin();
		advance(iterPath, iPathIndex);

		m_pGameInstance->Get_WorldPosition_Voxel(*iterPath, vCurVoxelPosVirtualPath);

		_float				fDistFromPre = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vCurVoxelPosVirtualPath) - XMLoadFloat3(&vPreVoxelPosVirtualPath))) };
		fAccDist += fDistFromPre;

		const auto&			CurRowAquireSucceed = AquiredSucceed[iPathIndex];
		const auto&			CurRowAquirePosees = AquiredPosesRows[iPathIndex];

		vector<_uint>		AquiredIndices;
		for (_uint iColIndex = 0; iColIndex < CurRowAquireSucceed.size(); ++iColIndex)
		{
			if (true == CurRowAquireSucceed[iColIndex])
				AquiredIndices.push_back(iColIndex);
		}
		const _int			iCurRowMinOffset = { static_cast<_int>(AquiredIndices.front() + min(iMinOffsetWidth, 0)) };
		const _int			iCurRowMaxOffset = { static_cast<_int>(AquiredIndices.back() + min(iMinOffsetWidth, 0)) };
		const _uint			iCurRowStartOffsetIndex = { AquiredIndices.front() };
		const _uint			iNumCurAquired = { static_cast<_uint>(iCurRowMaxOffset - iCurRowMinOffset) + 1 };

		const _float		fRatioAquired = { static_cast<_float>(iNumCurAquired) / static_cast<_float>(iRangeWidth + 1) };
		
		for (_uint iColIndex = 0; iColIndex <= iRangeWidth; ++iColIndex)
		{
			const _uint			iCurColIndex = { static_cast<_uint>(round(iColIndex * fRatioAquired)) + iCurRowStartOffsetIndex };
			_float3				vCurColPos = { CurRowAquirePosees[iCurColIndex] };
			
			_uint				iCurColVoxelIndex;
			if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurColPos, iCurColVoxelIndex)))
			{
				MSG_BOX(TEXT("Faield To GetIndex = > FormationManager : "));
				return E_FAIL;
			}

			_float			fRatio = { fAccDist / m_GroupInfos[iGroupIndexTag]->fVirtualPathTotalLength };
			auto			iterColRatio = m_GroupInfos[iGroupIndexTag]->ColsRatios[iColIndex].find(iCurColVoxelIndex);
			if (iterColRatio != m_GroupInfos[iGroupIndexTag]->ColsRatios[iColIndex].end())
			{
				if (iterColRatio->second < fRatio)
					iterColRatio->second = fRatio;
			}
			else
				m_GroupInfos[iGroupIndexTag]->ColsRatios[iColIndex].insert({ iCurColVoxelIndex, fRatio });
			CurPathCols[iColIndex] = iCurColVoxelIndex;
		}
	}

	//	3.
	//	만들어진 경로에 대한 검증 및 보수
	//	경로를 각 검사하여 이전노드와 다음노드가 이웃하지 않은경우 국소적 길찾기를 적용하여 경로를 잇는다. 
	_float		fOriginWeight = { m_pGameInstance->Get_HeuristicWeights(HEURISTIC_TYPE::_OPENNESS) };
	m_pGameInstance->Set_HeuristicWeights(HEURISTIC_TYPE::_OPENNESS, 0.f);

	vector<list<_uint>>		ColsPathes;
	ColsPathes.resize(static_cast<size_t>(iRangeWidth) + 1);

	for (_uint iColIndex = 0; iColIndex <= iRangeWidth; ++iColIndex)
	{
		list<_uint>			ResultPathIndices;
		_float3				vTempFloat3;

		auto				iterPathesRows = PathesRows.begin();
		_uint				iCurVoxelIndex = { (*iterPathesRows)[iColIndex] };
		_uint				iPreVoxelIndex = {};
		++iterPathesRows;

		_vector				vCurVoxelPos, vPreVoxelPos;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vTempFloat3)))
			return E_FAIL;
		vCurVoxelPos = XMLoadFloat3(&vTempFloat3);

		auto				iterGuidePath = m_GroupInfos[iGroupIndexTag]->VirtualPath.begin();
		_uint				iCurGuideVoxelIndex = { (*iterGuidePath) };
		_uint				iPreGuideVoxelIndex = {};
		++iterGuidePath;

		_vector				vCurGuideVoxelPos, vPreGuideVoxelPos;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurGuideVoxelIndex, vTempFloat3)))
			return E_FAIL;
		vCurGuideVoxelPos = XMLoadFloat3(&vTempFloat3);


		ResultPathIndices.push_back(iCurVoxelIndex);

		while (PathesRows.end() != iterPathesRows)
		{
			iPreVoxelIndex = ResultPathIndices.back();
			iCurVoxelIndex = (*iterPathesRows)[iColIndex];
			++iterPathesRows;

			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iPreVoxelIndex, vTempFloat3)))
				return E_FAIL;
			vPreVoxelPos = XMLoadFloat3(&vTempFloat3);

			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vTempFloat3)))
				return E_FAIL;
			vCurVoxelPos = XMLoadFloat3(&vTempFloat3);

			iPreGuideVoxelIndex = iCurGuideVoxelIndex;
			iCurGuideVoxelIndex = (*iterGuidePath);
			++iterGuidePath;

			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurGuideVoxelIndex, vTempFloat3)))
				return E_FAIL;
			vPreGuideVoxelPos = vCurGuideVoxelPos;
			vCurGuideVoxelPos = XMLoadFloat3(&vTempFloat3);

			//	동일 복셀 => 건너뜀 중복
			if (iPreVoxelIndex == iCurVoxelIndex)
				continue;

			//	방향성이 가이드 패스와 다른경우
			_vector			vDirectionFromPre = { vCurVoxelPos - vPreVoxelPos };
			_vector			vGuideDirectionFromPre = { vCurGuideVoxelPos - vPreGuideVoxelPos };

			_float			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(XMVectorSetY(vDirectionFromPre,0.f)), XMVector3Normalize(XMVectorSetY(vGuideDirectionFromPre,0.f)))) };
			if (XMConvertToRadians(30.f) > fDot)	//	방향성이 90도 이상 틀어진 경우...
				//	if (0.f > fDot)	//	방향성이 90도 이상 틀어진 경우...
				continue;

			//	이웃이 아닌경우 => 중간 경로 삽입 필요
			auto			NeighborIndices = m_pGameInstance->Get_NeighborIndices_Voxel(iCurVoxelIndex, VOXEL_LAYER::_STATIC);
			auto			iterNeighbor = find(NeighborIndices.begin(), NeighborIndices.end(), iPreVoxelIndex);

			if (NeighborIndices.end() == iterNeighbor)
			{
				list<_uint>		TempPathIndices;
				NODE_INFOS		NodeInfos;
				if (FAILED(m_pGameInstance->Make_Path_Voxel(TempPathIndices, NodeInfos, iPreVoxelIndex, { iCurVoxelIndex }, false, false, {})))
					continue;

				TempPathIndices.pop_front();
				TempPathIndices.pop_back();

				for (auto iVoxelIndex : TempPathIndices)
					ResultPathIndices.push_back(iVoxelIndex);
			}

			ResultPathIndices.push_back(iCurVoxelIndex);
		}

		//	이전경로와 다다음 경로가 직접 이웃일 경우 다음경로를 제거하여 중간최적화 => 상 + 좌 => 상좌
		auto			iter = ResultPathIndices.begin();
		_uint			iPreIndex = {};
		_uint			iCurIndex = { *(iter++) };
		_uint			iNextIndex = { *(iter++) };

		auto			iterLast = ResultPathIndices.end();
		--iterLast;

		_uint			iDeleteCnt{};
		while (iter != iterLast)
		{
			iPreIndex = iCurIndex;
			iCurIndex = iNextIndex;
			iNextIndex = { *(iter++) };

			auto		PreNeighborIndices = m_pGameInstance->Get_NeighborIndices_Voxel(iPreIndex, VOXEL_LAYER::_STATIC);			
			auto		iterNeighbor = find(PreNeighborIndices.begin(), PreNeighborIndices.end(), iNextIndex);
			
			if (iterNeighbor != PreNeighborIndices.end())
			{
				auto		iterDelete = find(ResultPathIndices.begin(), ResultPathIndices.end(), iCurIndex);
				ResultPathIndices.erase(iterDelete);

				iCurIndex = iPreIndex;

				++iDeleteCnt;
			}
		}
		cout << iDeleteCnt << endl;

		if (FAILED(Optimize_Path_Duplication(ResultPathIndices)))
			return E_FAIL;

		ColsPathes[iColIndex] = move(ResultPathIndices);
	}
	m_pGameInstance->Set_HeuristicWeights(HEURISTIC_TYPE::_OPENNESS, fOriginWeight);


	//	각 열의 패스최적화 이후 재생도 정리
	auto&			ColsRatios = m_GroupInfos[iGroupIndexTag]->ColsRatios;
	for (_uint i = 0; i < iRangeWidth + 1; ++i)
	{
		auto&			CurColsRatios = ColsRatios[i];
		const auto&		CurColPathIndices = ColsPathes[i];
		auto			iter = CurColPathIndices.begin();
		auto			iterStart = CurColPathIndices.begin();

		while(iter != CurColPathIndices.end())
		{
			const _uint			iPathVoxelIndex = { (*iter) };
			const _bool			isExist = { CurColsRatios.end() != CurColsRatios.find(iPathVoxelIndex) };
			
			if (true == isExist &&
				iterStart != iter)
			{
				const _float		fStartRatio = { CurColsRatios.find(*iterStart)->second };
				const _float		fEndRatio = { CurColsRatios.find(*iter)->second };
				const _float		fRatioRange = { fEndRatio - fStartRatio };

				list<_uint>			BetweenIndices;

				const _uint			iStartVoxelIndex = { *iterStart };

				while (true)
				{
					BetweenIndices.push_back(*iterStart);
					++iterStart;
					if (iterStart == iter)
						break;
				}
				
				_float			fTotalDistBetween = {};
				if (FAILED(Compute_TotalLength_Path(BetweenIndices, fTotalDistBetween)))
					continue;

				_float			fAccDist{};

				for (_uint i = 1; i < static_cast<_uint>(BetweenIndices.size() - 1); ++i)
				{
					auto			iterBetween = BetweenIndices.begin();
					advance(iterBetween, i);

					const _uint		iBetweenVoxelIndex = {*iterBetween };
					const _uint		iBetweenPreVoxelIndex = { *(--iterBetween) };

					_float3			vBetweenVoxelPos, vBetweenPreVoxelPos;
					if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iBetweenVoxelIndex, vBetweenVoxelPos)) ||
						FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iBetweenPreVoxelIndex, vBetweenPreVoxelPos)))
						return E_FAIL;

					const _float	fDistFromPre = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vBetweenVoxelPos) - XMLoadFloat3(&vBetweenPreVoxelPos))) };
					fAccDist += fDistFromPre;

					const _float	fCurRatio{ fAccDist / fTotalDistBetween };
					const _float	fReulstRatio = { fStartRatio + (fCurRatio * fRatioRange) };

					CurColsRatios[iBetweenVoxelIndex] = fReulstRatio;
				}
			}

			++iter;
		}
	}

	//	에이전트들을 가장 가까운 시작지위치로 배정할 수 있도록 정렬함
	vector<CFormation*>			Formations = { move(m_GroupInfos[iGroupIndexTag]->Formations) };
	
	for (_uint iFormationIndex = 0; iFormationIndex < m_iNumAgent; ++iFormationIndex)
	{
		const _int			iOffset_Depth = { Offset_Depth[iFormationIndex] };
		const _int			iOffset_Width = { Offset_Width[iFormationIndex] };

		const _uint			iColIndex = { static_cast<_uint>(iOffset_Width - iMinOffsetWidth) };
		const _uint			iRowIndexOffset = { static_cast<_uint>(iOffset_Depth - iMinOffsetDepth) };

		//	auto iter = m_GroupInfos[iGroupIndexTag]->ColsPathes[iColIndex].begin();
		auto iter = ColsPathes[iColIndex].begin();
		advance(iter, iRowIndexOffset);

		const _uint			iCurFormationVoxelIndex = { *iter };

		_float3				vCurFormationPosFloat3;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurFormationVoxelIndex, vCurFormationPosFloat3)))
			return E_FAIL;
		_vector				vFormationPos = { XMLoadFloat3(&vCurFormationPosFloat3) };

		_uint			iMostNearAgentIndex = {};
		_float			fNearDist = { FLT_MAX };
		for (_uint iAgentIndex = 0; iAgentIndex < Formations.size(); ++iAgentIndex)
		{
			CTransform* pTransform = { Formations[iAgentIndex]->Get_Transform() };
			if (nullptr == pTransform)
				return E_FAIL;

			_vector				vAgentPos = { pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
			const _float		fDist = { XMVectorGetX(XMVector3Length(vFormationPos - vAgentPos)) };

			if (fDist < fNearDist)
			{
				iMostNearAgentIndex = iAgentIndex;
				fNearDist = fDist;
			}
		}

		m_GroupInfos[iGroupIndexTag]->Formations.push_back(Formations[iMostNearAgentIndex]);
		auto		iterAgent = Formations.begin();
		advance(iterAgent, iMostNearAgentIndex);
		Formations.erase(iterAgent);
	}
	

	//	최종경로 기입 => 에이전트 별로
	m_GroupInfos[iGroupIndexTag]->ColsPathes.resize(static_cast<size_t>(iRangeWidth) + 1);

	for (_uint i = 0; i <= iRangeWidth; ++i)
		m_GroupInfos[iGroupIndexTag]->ColsPathes[i] = move(ColsPathes[i]);

	for (_uint i = 0; i < iNumAgent; ++i)
	{
		const _int			iCurAgentOffsetWidth = { Offset_Width[i] };
		const _int			iCurAgentOffsetDepth = { Offset_Depth[i] };

		const _uint			iNumCutFront = { static_cast<_uint>(iCurAgentOffsetDepth - iMinOffsetDepth) };
		const _uint			iNumCutBack = { static_cast<_uint>(iMaxOffsetDepth - iCurAgentOffsetDepth) };

		const _uint			iCurAgentColIndex = { static_cast<_uint>(iCurAgentOffsetWidth - iMinOffsetWidth) };

		list<_uint>			CurAgentPath = { m_GroupInfos[iGroupIndexTag]->ColsPathes[iCurAgentColIndex] };

		_float				fCutDist = {};
		for (_uint i = 0; i < iNumCutFront && false == CurAgentPath.empty(); ++i)
		{
			auto		iter = VirtualPath.begin();
			advance(iter, i);
			_uint		iCurVoxelIndex = { *iter };
			_uint		iNextVoxelIndex = { *(++iter) };

			_float3		vCurPos, vNextPos;
			if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vCurPos)) ||
				FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iNextVoxelIndex, vNextPos)))
				return E_FAIL;

			_float		fDistToNext = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vNextPos) - XMLoadFloat3(&vCurPos))) };
			fCutDist += fDistToNext;
			
			CurAgentPath.pop_front();
		}
		for (_uint i = 0; i < iNumCutBack && false == CurAgentPath.empty(); ++i)
			CurAgentPath.pop_back();

		const _float	fRatioOffset = { fCutDist / m_GroupInfos[iGroupIndexTag]->fVirtualPathTotalLength };


		CTransform*			pTransform = { m_GroupInfos[iGroupIndexTag]->Formations[i]->Get_Transform() };
		if (nullptr == pTransform)
			return E_FAIL;

		_vector				vPosition = { pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
		_float3				vPositionFloat3;
		XMStoreFloat3(&vPositionFloat3, vPosition);

		unordered_map<_uint, _float>			CurRatios = { m_GroupInfos[iGroupIndexTag]->ColsRatios[iCurAgentColIndex] };
		_uint				iAgentVoxelIndex;
		if (!(FAILED(m_pGameInstance->Get_Index_Voxel(vPositionFloat3, iAgentVoxelIndex))))
		{
			list<_uint>			BestList;
			NODE_INFOS			NodeInfos;
			vector<_uint>		GoalIndices;
			for (auto iVoxelIndex : CurAgentPath)
				GoalIndices.push_back(iVoxelIndex);

			if (!FAILED(m_pGameInstance->Make_Path_Voxel(BestList, NodeInfos, iAgentVoxelIndex, GoalIndices, false, false, {})))
			{
				auto		iterBestList = BestList.begin();
				auto		iterCurPath = CurAgentPath.end();
				while (iterBestList != BestList.end())
				{
					const _uint			iCurVoxelIndex = { *iterBestList };

					iterCurPath = find(CurAgentPath.begin(), CurAgentPath.end(), iCurVoxelIndex);
					if (iterCurPath != CurAgentPath.end())
					{
						while (iterBestList != BestList.end())
							iterBestList = BestList.erase(iterBestList);
					}
					else
						++iterBestList;
				}

				_uint			iTempStartIndex = { *iterCurPath };
				_float			fLastRatio = { m_GroupInfos[iGroupIndexTag]->ColsRatios[iCurAgentColIndex][iTempStartIndex] };

				_float			fDistFrontPath;
				if (FAILED(Compute_TotalLength_Path(BestList, fDistFrontPath)))
					return E_FAIL;

				_float			fAccDist = {};
				for (_uint i = 1; i < BestList.size(); ++i)
				{
					auto		iter = BestList.begin();
					advance(iter, i);
					_uint		iCurVoxelIndex = { *iter };
					_uint		iPreVoxelIndex = { *(--iter) };

					_float3		vCurPos, vPrePos;
					if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vCurPos)) ||
						FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iPreVoxelIndex, vPrePos)))
						return E_FAIL;

					const _float		fDistFromPre = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vCurPos) - XMLoadFloat3(&vPrePos))) };
					fAccDist += fDistFromPre;

					const _float			fCurRatio = { fAccDist / fDistFrontPath };
					const _float			fResultRatio = { fCurRatio * fLastRatio };

					CurRatios[iCurVoxelIndex] = fResultRatio;
				}

				if (iterCurPath != CurAgentPath.end())
				{
					auto		iterLast = iterCurPath;
					auto		iterStart = CurAgentPath.begin();
					while (iterStart != iterLast)
					{
						iterStart = CurAgentPath.erase(iterStart);
					}
				}

				reverse(BestList.begin(), BestList.end());

				for (auto iVoxelIndex : BestList)
					CurAgentPath.push_front(iVoxelIndex);
			}

			/*if (!(FAILED(m_pGameInstance->Convert_BestList_From_FlowField(BestList, m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag, iAgentVoxelIndex))) &&
				false == BestList.empty())
			{
				auto		iterBestList = BestList.begin();
				auto		iterCurPath = CurAgentPath.end();
				auto		iterGuidePath = VirtualPath.end();
				while (iterBestList != BestList.end())
				{
					const _uint			iCurVoxelIndex = { *iterBestList };

					iterGuidePath = find(VirtualPath.begin(), VirtualPath.end(), iCurVoxelIndex);
					iterCurPath = find(CurAgentPath.begin(), CurAgentPath.end(), iCurVoxelIndex);
					if (iterCurPath != CurAgentPath.end())
					{
						while (iterBestList != BestList.end())
							iterBestList = BestList.erase(iterBestList);
					}
					else if (iterGuidePath != VirtualPath.end())
					{
						while (iterBestList != BestList.end())
							iterBestList = BestList.erase(iterBestList);

						_uint			iNewStartIndex = { BestList.back() };
						list<_uint>		AdditionalPath;
						NODE_INFOS		NodeInfos;

						vector<_uint>		GoalIndices;
						for (auto iVoxelIndex : CurAgentPath)
							GoalIndices.push_back(iVoxelIndex);

						m_pGameInstance->Make_Path_Voxel(AdditionalPath, NodeInfos, iNewStartIndex, GoalIndices, false, false, {});

						for (auto iVoxelIndex : AdditionalPath)
							BestList.push_back(iVoxelIndex);

						iterCurPath = find(CurAgentPath.begin(), CurAgentPath.end(), BestList.back());
						BestList.pop_back();
					}
					else
						++iterBestList;
				}

				_uint			iTempStartIndex = { *iterCurPath };
				_float			fLastRatio = { m_GroupInfos[iGroupIndexTag]->ColsRatios[iCurAgentColIndex][iTempStartIndex] };

				_float			fDistFrontPath;
				if (FAILED(Compute_TotalLength_Path(BestList, fDistFrontPath)))
					return E_FAIL;

				_float			fAccDist = {};
				for (_uint i = 1; i < BestList.size(); ++i)
				{
					auto		iter = BestList.begin();
					advance(iter, i);
					_uint		iCurVoxelIndex = { *iter };
					_uint		iPreVoxelIndex = { *(--iter) };

					_float3		vCurPos, vPrePos;
					if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vCurPos)) ||
						FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iPreVoxelIndex, vPrePos)))
						return E_FAIL;

					const _float		fDistFromPre = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vCurPos) - XMLoadFloat3(&vPrePos))) };
					fAccDist += fDistFromPre;

					const _float			fCurRatio = { fAccDist / fDistFrontPath };
					const _float			fResultRatio = { fCurRatio * fLastRatio };

					CurRatios[iCurVoxelIndex] = fResultRatio;
				}

				if (iterCurPath != CurAgentPath.end())
				{
					auto		iterLast = iterCurPath;
					auto		iterStart = CurAgentPath.begin();
					while (iterStart != iterLast)
					{
						iterStart = CurAgentPath.erase(iterStart);
					}
				}

				reverse(BestList.begin(), BestList.end());

				for (auto iVoxelIndex : BestList)
					CurAgentPath.push_front(iVoxelIndex);
			}*/
		}

		m_GroupInfos[iGroupIndexTag]->RatioOffsets[i] = fRatioOffset;
		m_GroupInfos[iGroupIndexTag]->Formations[i]->Set_BestPathIndices(CurAgentPath);
		m_GroupInfos[iGroupIndexTag]->Formations[i]->Set_PathRatios(move(CurRatios));
	}

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_Virtual_Path(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;
	
	list<_uint>			BestList;
	NODE_INFOS			NodeInfos;

	if (FAILED(m_pGameInstance->Make_Path_Voxel(BestList, NodeInfos, iStartIndex, { iGoalIndex }, false, false, {})))
		return E_FAIL;

	_float3				vGoalPosFloat3, vGoalPrePosFloat3;
	auto				iterGoalPre = BestList.end();
	--iterGoalPre; --iterGoalPre;
	_uint				iGoalPreIndex = { *iterGoalPre };
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iGoalPreIndex, vGoalPrePosFloat3)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iGoalIndex, vGoalPosFloat3)))
		return E_FAIL;

	_vector				vDirFromPre = { XMLoadFloat3(&vGoalPosFloat3) - XMLoadFloat3(&vGoalPrePosFloat3) };
	_vector				vGoalPos = { XMLoadFloat3(&vGoalPosFloat3) };

	const _float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };
	const _float		fOffsetY[] = { 0.f, -fVoxelSize, fVoxelSize };
	const _uint			iNumOffsetY = { sizeof(fOffsetY) / sizeof(_float) };

	_float3					vCurOffsetPosFloat3 = { vGoalPosFloat3 };

	//	머리 추가 도착지...
	for (_int i = 0; i < m_iMaxOffsetDepth; ++i)
	{
		_vector			vCurOffsetPos = { XMLoadFloat3(&vCurOffsetPosFloat3) + vDirFromPre };
		XMStoreFloat3(&vCurOffsetPosFloat3, vCurOffsetPos);

		for (_uint j = 0; j < iNumOffsetY; ++j)
		{
			VOXEL_ID		eVoxelID;
			_float3			vCurOffsetPosAppliedY = { vCurOffsetPosFloat3 };
			vCurOffsetPosAppliedY.y += fOffsetY[j];

			if (!FAILED(m_pGameInstance->Get_VoxelID(vCurOffsetPosAppliedY, eVoxelID, VOXEL_LAYER::_STATIC)) &&
				VOXEL_ID::_FLOOR == eVoxelID)
			{
				vCurOffsetPosFloat3 = vCurOffsetPosAppliedY;

				_uint		iVoxelIndex = {};
				if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurOffsetPosAppliedY, iVoxelIndex)))
					return E_FAIL;

				BestList.push_back(iVoxelIndex);
				break;
			}
		}
	}

	_float3				vStartPosFloat3, vStartNextPosFloat3;
	auto				iterStartNext = BestList.begin();
	iterStartNext++;
	_uint				iStarNextIndex = { *iterStartNext };
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iStarNextIndex, vStartNextPosFloat3)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iStartIndex, vStartPosFloat3)))
		return E_FAIL;

	_vector				vDirFromNext = { XMLoadFloat3(&vStartPosFloat3) - XMLoadFloat3(&vStartNextPosFloat3) };
	_vector				vStartPos = { XMLoadFloat3(&vStartPosFloat3) };

	vCurOffsetPosFloat3 = vStartPosFloat3;
	//	꼬리 추가 출발지....
	for (_int i = 0; i > m_iMinOffsetDepth; --i)
	{
		_vector			vCurOffsetPos = { XMLoadFloat3(&vCurOffsetPosFloat3) + vDirFromNext };
		XMStoreFloat3(&vCurOffsetPosFloat3, vCurOffsetPos);

		for (_uint j = 0; j < iNumOffsetY; ++j)
		{
			VOXEL_ID		eVoxelID;
			_float3			vCurOffsetPosAppliedY = { vCurOffsetPosFloat3 };
			vCurOffsetPosAppliedY.y += fOffsetY[j];

			if (!FAILED(m_pGameInstance->Get_VoxelID(vCurOffsetPosAppliedY, eVoxelID, VOXEL_LAYER::_STATIC)) &&
				VOXEL_ID::_FLOOR == eVoxelID)
			{
				vCurOffsetPosFloat3 = vCurOffsetPosAppliedY;

				_uint		iVoxelIndex = {};
				if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurOffsetPosAppliedY, iVoxelIndex)))
					return E_FAIL;

				BestList.push_front(iVoxelIndex);
				break;
			}
		}
	}

	if (FAILED(Compute_TotalLength_Path(BestList, m_GroupInfos[iGroupIndexTag]->fVirtualPathTotalLength)))
		return E_FAIL;


	m_GroupInfos[iGroupIndexTag]->VirtualPath = move(BestList);

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_Virtual_Path_FlowField(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;

	list<_uint>			BestList;
	vector<_uint>		StartIndices;

	StartIndices.push_back(iStartIndex);
	for (auto pFormation : m_GroupInfos[iGroupIndexTag]->Formations)
	{
		CTransform*			pTransform = { pFormation->Get_Transform() };
		if (nullptr == pTransform)
			return E_FAIL;

		_vector				vPosition = { pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
		_float3				vPosFloat3;
		XMStoreFloat3(&vPosFloat3, vPosition);

		_uint				iVoxelIndex;
		if (FAILED(m_pGameInstance->Get_Index_Voxel(vPosFloat3, iVoxelIndex)))
			continue;

		StartIndices.push_back(iVoxelIndex);
	}

	if (FAILED(m_pGameInstance->Make_VectorFiled_A_Star(iGoalIndex, StartIndices, 1, m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Convert_BestList_From_FlowField(BestList, m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag, iStartIndex)))
		return E_FAIL;

	_float3				vGoalPosFloat3, vGoalPrePosFloat3;
	auto				iterGoalPre = BestList.end();
	--iterGoalPre; --iterGoalPre;
	_uint				iGoalPreIndex = { *iterGoalPre };
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iGoalPreIndex, vGoalPrePosFloat3)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iGoalIndex, vGoalPosFloat3)))
		return E_FAIL;

	_vector				vDirFromPre = { XMLoadFloat3(&vGoalPosFloat3) - XMLoadFloat3(&vGoalPrePosFloat3) };
	_vector				vGoalPos = { XMLoadFloat3(&vGoalPosFloat3) };

	const _float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };
	const _float		fOffsetY[] = { 0.f, -fVoxelSize, fVoxelSize };
	const _uint			iNumOffsetY = { sizeof(fOffsetY) / sizeof(_float) };

	_float3					vCurOffsetPosFloat3 = { vGoalPosFloat3 };
	/*

	////	머리 추가 도착지...
	//for (_int i = 0; i < m_iMaxOffsetDepth; ++i)
	//{
	//	_vector			vCurOffsetPos = { XMLoadFloat3(&vCurOffsetPosFloat3) + vDirFromPre };
	//	XMStoreFloat3(&vCurOffsetPosFloat3, vCurOffsetPos);

	//	for (_uint j = 0; j < iNumOffsetY; ++j)
	//	{
	//		VOXEL_ID		eVoxelID;
	//		_float3			vCurOffsetPosAppliedY = { vCurOffsetPosFloat3 };
	//		vCurOffsetPosAppliedY.y += fOffsetY[j];

	//		if (!FAILED(m_pGameInstance->Get_VoxelID(vCurOffsetPosAppliedY, eVoxelID, VOXEL_LAYER::_STATIC)) &&
	//			VOXEL_ID::_FLOOR == eVoxelID)
	//		{
	//			vCurOffsetPosFloat3 = vCurOffsetPosAppliedY;

	//			_uint		iVoxelIndex = {};
	//			if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurOffsetPosAppliedY, iVoxelIndex)))
	//				return E_FAIL;

	//			BestList.push_back(iVoxelIndex);
	//			break;
	//		}
	//	}
	//}

	//_float3				vStartPosFloat3, vStartNextPosFloat3;
	//auto				iterStartNext = BestList.begin();
	//iterStartNext++;
	//_uint				iStarNextIndex = { *iterStartNext };
	//if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iStarNextIndex, vStartNextPosFloat3)) ||
	//	FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iStartIndex, vStartPosFloat3)))
	//	return E_FAIL;

	//_vector				vDirFromNext = { XMLoadFloat3(&vStartPosFloat3) - XMLoadFloat3(&vStartNextPosFloat3) };
	//_vector				vStartPos = { XMLoadFloat3(&vStartPosFloat3) };

	//vCurOffsetPosFloat3 = vStartPosFloat3;
	////	꼬리 추가 출발지....
	//for (_int i = 0; i > m_iMinOffsetDepth; --i)
	//{
	//	_vector			vCurOffsetPos = { XMLoadFloat3(&vCurOffsetPosFloat3) + vDirFromNext };
	//	XMStoreFloat3(&vCurOffsetPosFloat3, vCurOffsetPos);

	//	for (_uint j = 0; j < iNumOffsetY; ++j)
	//	{
	//		VOXEL_ID		eVoxelID;
	//		_float3			vCurOffsetPosAppliedY = { vCurOffsetPosFloat3 };
	//		vCurOffsetPosAppliedY.y += fOffsetY[j];

	//		if (!FAILED(m_pGameInstance->Get_VoxelID(vCurOffsetPosAppliedY, eVoxelID, VOXEL_LAYER::_STATIC)) &&
	//			VOXEL_ID::_FLOOR == eVoxelID)
	//		{
	//			vCurOffsetPosFloat3 = vCurOffsetPosAppliedY;

	//			_uint		iVoxelIndex = {};
	//			if (FAILED(m_pGameInstance->Get_Index_Voxel(vCurOffsetPosAppliedY, iVoxelIndex)))
	//				return E_FAIL;

	//			BestList.push_front(iVoxelIndex);
	//			break;
	//		}
	//	}
	//}*/

	if (FAILED(Compute_TotalLength_Path(BestList, m_GroupInfos[iGroupIndexTag]->fVirtualPathTotalLength)))
		return E_FAIL;


	m_GroupInfos[iGroupIndexTag]->VirtualPath = move(BestList);

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_Path_Lengthes(const _uint iGroupIndexTag)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;

	vector<_float>		PathTotalLengthes;
	auto&				Formations = m_GroupInfos[iGroupIndexTag]->Formations;

	for (auto pFormation : Formations)
	{
		auto&		BestPathIndices = pFormation->Get_BestPathIndices();
		_float		fTotalDist = { Compute_Path_TotalDist(BestPathIndices) };

		if (0.f > fTotalDist)
			return E_FAIL;

		PathTotalLengthes.push_back(fTotalDist);
	}

	m_GroupInfos[iGroupIndexTag]->PathTotalLengthes = move(PathTotalLengthes);

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_Rollback_Virtual_Path(const _uint iGroupIndexTag)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;

	auto&			VirtualPath = m_GroupInfos[iGroupIndexTag]->VirtualPath;
	for (_int i = 0; i < m_iMaxOffsetDepth; ++i)
		VirtualPath.pop_back();
	for (_int i = 0; i > m_iMinOffsetDepth; --i)
		VirtualPath.pop_front();

	if (UINT_MAX != m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag)
	{
		if (FAILED(m_pGameInstance->Release_FlowField(m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CFormation_Manager::Get_Formation_Offsets(vector<_int>& Offset_Width, vector<_int>& Offset_Depth, const _uint iNumAgent) const
{
	Offset_Width.clear();
	Offset_Depth.clear();

	if (FORMATION_TYPE::_CIRCLE == m_eFormationType)
	{
		for(_uint i = 0; i < iNumAgent; ++i)
			Offset_Width.push_back(Formation_Circle_Offset_Width[i]);
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Depth.push_back(Formation_Circle_Offset_Depth[i]);
	}

	else if (FORMATION_TYPE::_TWO_LINE == m_eFormationType)
	{
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Width.push_back(Formation_TwoLine_Offset_Width[i]);
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Depth.push_back(Formation_TwoLine_Offset_Depth[i]);
	}

	else if(FORMATION_TYPE::_FOUR_LINE == m_eFormationType)
	{
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Width.push_back(Formation_FourLine_Offset_Width[i]);
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Depth.push_back(Formation_FourLine_Offset_Depth[i]);
	}

	else if (FORMATION_TYPE::_EIGHT_LINE == m_eFormationType)
	{
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Width.push_back(Formation_EightLine_Offset_Width[i]);
		for (_uint i = 0; i < iNumAgent; ++i)
			Offset_Depth.push_back(Formation_EightLine_Offset_Depth[i]);
	}

	return S_OK;
}

HRESULT CFormation_Manager::Compute_Aquired_Range(const _int iMinOffsetWidth, const _int iMaxOffsetWidth, list<_uint>& VirtualPathIndices, vector<vector< _float3>>& AquiredPoses, vector<vector< _bool>>& AquiredSucceed)
{
	const _float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };

	_vector				vWorldLookDir = { XMVectorSet(0.f, 0.f, 1.f, 0.f) };
	_vector				vWorldRightDir = { XMVectorSet(1.f, 0.f, 0.f, 0.f) };

	auto				iter = VirtualPathIndices.begin();
	auto				iterLast = VirtualPathIndices.end();
	--iterLast;

	_uint				iVoxelIndexGuide_Cur = {  };
	_uint				iVoxelIndexGuide_Next = { *(iter) };

	_float3				vGuideVoxelPrePosFloat3, vGuideVoxelCurPosFloat3, vGuideVoxelNextPosFloat3;
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndexGuide_Cur, vGuideVoxelCurPosFloat3)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndexGuide_Next, vGuideVoxelNextPosFloat3)))
		return E_FAIL;

	const _uint			iCenterIndex = { static_cast<_uint>(iMinOffsetWidth < 0 ? abs(iMinOffsetWidth) : 0) };

	while (iter != VirtualPathIndices.end())
	{
		iVoxelIndexGuide_Cur = iVoxelIndexGuide_Next;
		if (iter != iterLast)
		iVoxelIndexGuide_Next = *(++iter);
		else
			++iter;

		vGuideVoxelPrePosFloat3 = vGuideVoxelCurPosFloat3;
		vGuideVoxelCurPosFloat3 = vGuideVoxelNextPosFloat3;

		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndexGuide_Next, vGuideVoxelNextPosFloat3)))
			return E_FAIL;

		_vector				vGuideVoxelPrePos = { XMLoadFloat3(&vGuideVoxelPrePosFloat3) };
		_vector				vGuideVoxelCurPos = { XMLoadFloat3(&vGuideVoxelCurPosFloat3) };
		_vector				vGuideVoxelNextPos = { XMLoadFloat3(&vGuideVoxelNextPosFloat3) };



		_vector				vDirection;
		if (VirtualPathIndices.end() == iter)
			vDirection = { vGuideVoxelCurPos - vGuideVoxelPrePos };
		else
			vDirection = { vGuideVoxelNextPos - vGuideVoxelCurPos };

		_vector				vDirToNextXZ = { XMVector3Normalize(XMVectorSetY(vDirection, 0.f)) };
		const _float		fDot = { XMVectorGetX(XMVector3Dot(vWorldLookDir, XMVector3Normalize(vDirToNextXZ))) };
		const _float		fAngle = { acosf(fDot) };

		_matrix				RotateMatrix = { XMMatrixIdentity() };
		//	Angle 0 ~ 180 정상 범위
		if (fAngle > EPSILON &&
			fAngle < XM_PI - EPSILON)
		{
			_vector				vRotateAxis = { XMVector3Cross(vWorldLookDir, XMVector3Normalize(vDirToNextXZ)) };
			_vector				vQuaternion = { XMQuaternionRotationAxis(vRotateAxis, fAngle) };
			RotateMatrix = XMMatrixRotationQuaternion(vQuaternion);
		}
		//	Angle 180 => 반대편을 봐야함
		else if (fAngle >= XM_PI - EPSILON)
		{
			_vector				vRotateAxis = { XMVectorSet(0.f, 1.f, 0.f, 0.f) };
			_vector				vQuaternion = { XMQuaternionRotationAxis(vRotateAxis, fAngle) };
			RotateMatrix = XMMatrixRotationQuaternion(vQuaternion);
		}

		_vector				vCurRight = { XMVector3TransformNormal(vWorldRightDir, RotateMatrix) };
		const _float		fMaxDimentionLen = { fmaxf(fabsf(XMVectorGetX(vCurRight)), fmaxf(fabsf(XMVectorGetY(vCurRight)), fabsf(XMVectorGetZ(vCurRight)))) };
		/* For. 대각선도 하나의 방향으로 인식하게 하는 방식*/
		//	vCurRight /= fMaxDimentionLen;
		vCurRight *= fVoxelSize;

		const _uint			iTotalRangeWidth = { static_cast<_uint>((iMaxOffsetWidth > 0 ? iMaxOffsetWidth : 0) - iMinOffsetWidth) };
		vector<_float3>		CurAquirePoses;
		vector<_bool>		isAquired;
		CurAquirePoses.resize(static_cast<size_t>(iTotalRangeWidth) + 1);
		isAquired.resize(static_cast<size_t>(iTotalRangeWidth) + 1);

		//	센터 삽입
		CurAquirePoses[iCenterIndex] = vGuideVoxelCurPosFloat3;
		isAquired[iCenterIndex] = true;

		//	좌측 경계 체크		
		const _float			OffsetY[] = { 0.f, -fVoxelSize, fVoxelSize };
		static const _uint		iNumOffsetY = { sizeof(OffsetY) / sizeof(_float) };

		_float3					vCurOffsetPosFloat3;
		XMStoreFloat3(&vCurOffsetPosFloat3, vGuideVoxelCurPos);

		for (_int i = -1; i >= iMinOffsetWidth; --i)
		{
			_vector			vCurOffsetPos = { XMLoadFloat3(&vCurOffsetPosFloat3) - vCurRight };
			XMStoreFloat3(&vCurOffsetPosFloat3, vCurOffsetPos);

			_bool				isFind = {};
			const _uint			iCurOffsetIndex = { static_cast<_uint>(i - iMinOffsetWidth) };

			for (_uint j = 0; j < iNumOffsetY; ++j)
			{
				VOXEL_ID		eVoxelID;
				_float3			vCurOffsetPosAppliedY = { vCurOffsetPosFloat3 };
				vCurOffsetPosAppliedY.y += OffsetY[j];

				if (!FAILED(m_pGameInstance->Get_VoxelID(vCurOffsetPosAppliedY, eVoxelID, VOXEL_LAYER::_STATIC)) &&
					VOXEL_ID::_FLOOR == eVoxelID)
				{
					CurAquirePoses[iCurOffsetIndex] = vCurOffsetPosAppliedY;
					isAquired[iCurOffsetIndex] = true;

					vCurOffsetPosFloat3 = vCurOffsetPosAppliedY;
					isFind = true;
					break;
				}
			}

			if (false == isFind)
				break;
		}

		XMStoreFloat3(&vCurOffsetPosFloat3, vGuideVoxelCurPos);

		//	우측 경계 체크
		for (_int i = 1; i <= iMaxOffsetWidth; ++i)
		{
			_vector			vCurOffsetPos = { XMLoadFloat3(&vCurOffsetPosFloat3) + vCurRight };
			XMStoreFloat3(&vCurOffsetPosFloat3, vCurOffsetPos);

			_bool				isFind = {};
			const _uint			iCurOffsetIndex = { static_cast<_uint>(i - iMinOffsetWidth) };

			for (_uint j = 0; j < iNumOffsetY; ++j)
			{
				VOXEL_ID		eVoxelID;
				_float3			vCurOffsetPosAppliedY = { vCurOffsetPosFloat3 };
				vCurOffsetPosAppliedY.y += OffsetY[j];

				if (!FAILED(m_pGameInstance->Get_VoxelID(vCurOffsetPosAppliedY, eVoxelID, VOXEL_LAYER::_STATIC)) &&
					VOXEL_ID::_FLOOR == eVoxelID)
				{
					CurAquirePoses[iCurOffsetIndex] = vCurOffsetPosAppliedY;
					isAquired[iCurOffsetIndex] = true;

					vCurOffsetPosFloat3 = vCurOffsetPosAppliedY;
					isFind = true;
					break;
				}
			}

			if (false == isFind)
				break;
		}

		AquiredPoses.push_back(move(CurAquirePoses));
		AquiredSucceed.push_back(move(isAquired));
	}

	return S_OK;
}

HRESULT CFormation_Manager::Compute_Formation_Offsets(const vector<_int>& Offset_Width, const vector<_int>& Offset_Depth, const _uint iNumAgent, _int& iMinOffsetWidth, _int& iMaxOffsetWidth, _int& iMinOffsetDepth, _int& iMaxOffsetDepth)
{
	const _float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };

	_int		iResultMinOffsetWidth{ INT_MAX }, iResultMaxOffsetWidth{ INT_MIN };
	_int		iResultMinOffsetDepth{ INT_MAX }, iResultMaxOffsetDepth{ INT_MIN };

	for (_uint i = 0; i < iNumAgent; ++i)
	{
		const _int			iCurOffsetWidth = Offset_Width[i];
		iResultMinOffsetWidth = min(iResultMinOffsetWidth, iCurOffsetWidth);
		iResultMaxOffsetWidth = max(iResultMaxOffsetWidth, iCurOffsetWidth);
	}

	for (_uint i = 0; i < iNumAgent; ++i)
	{
		const _int			iCurOffsetDepth = Offset_Depth[i];
		iResultMinOffsetDepth = min(iResultMinOffsetDepth, iCurOffsetDepth);
		iResultMaxOffsetDepth = max(iResultMaxOffsetDepth, iCurOffsetDepth);
	}

	iMinOffsetWidth = iResultMinOffsetWidth;
	iMaxOffsetWidth = iResultMaxOffsetWidth;
	iMinOffsetDepth = iResultMinOffsetDepth;
	iMaxOffsetDepth = iResultMaxOffsetDepth;

	return S_OK;
}

HRESULT CFormation_Manager::Compute_TotalLength_Path(const list<_uint>& PathIndices, _float& fTotalDist)
{
	fTotalDist = Compute_Path_TotalDist(PathIndices);
	return S_OK;
}

HRESULT CFormation_Manager::Optimize_Aquired_Voxels(vector<vector<_float3>>& AquiredPoses, vector<vector<_bool>>& AquiredSucceed, const _uint iCurIndex)
{
	//	변화폭 제한
	stack<_uint>	Stack;
	for(_int i = 0; i < AquiredSucceed.size(); ++i)
		Stack.push(i);

	while (false == Stack.empty())
	{
		const _uint		iTopIndex = { Stack.top() };
		Stack.pop();

		const _uint		iPreIndex = { iTopIndex > 0 ? iTopIndex - 1 : iTopIndex };
		const _uint		iNextIndex = { static_cast<_uint>(iTopIndex + 1) < AquiredSucceed.size() - 1 ? iTopIndex + 1 : iTopIndex };

		if (iPreIndex != iTopIndex)
		{
			_uint		iMinAquiredIndex_Pre = { UINT_MAX };
			_uint		iMinAquiredIndex_Top = { UINT_MAX };
			_uint		iMaxAquiredIndex_Pre = { 0 };
			_uint		iMaxAquiredIndex_Top = { 0 };

			for (_uint i = 0; i < static_cast<_uint>(AquiredSucceed[iTopIndex].size()); ++i)
			{
				if (true == AquiredSucceed[iTopIndex][i])
				{
					iMinAquiredIndex_Top = min(iMinAquiredIndex_Top, i);
					iMaxAquiredIndex_Top = max(iMaxAquiredIndex_Top, i);
				}
				if (true == AquiredSucceed[iPreIndex][i])
				{
					iMinAquiredIndex_Pre = min(iMinAquiredIndex_Pre, i);
					iMaxAquiredIndex_Pre = max(iMaxAquiredIndex_Pre, i);
				}
			}

			const _int		iDiffMin = { static_cast<_int>(iMinAquiredIndex_Pre) - static_cast<_int>(iMinAquiredIndex_Top) };
			if (abs(iDiffMin) > ms_iMaxDiffAquired)
			{
				if (0 > iDiffMin)		//	음수 => Pre가 Top 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMin) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iPreIndex][iMinAquiredIndex_Pre + i] = false;
					Stack.push(iPreIndex);

				}
				else					//	양수 => Top이 Pre 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMin) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iTopIndex][iMinAquiredIndex_Top + i] = false;
					Stack.push(iTopIndex);
				}
			}

			const _int		iDiffMax = { static_cast<_int>(iMaxAquiredIndex_Pre) - static_cast<_int>(iMaxAquiredIndex_Top) };
			if (abs(iDiffMax) > ms_iMaxDiffAquired)
			{
				if (0 < iDiffMax)		//	양수 => Pre가 Top 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMax) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iPreIndex][iMaxAquiredIndex_Pre - i] = false;
					Stack.push(iPreIndex);

				}
				else					//	음수 => Top이 Pre 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMax) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iTopIndex][iMaxAquiredIndex_Top - i] = false;
					Stack.push(iTopIndex);
				}
			}
		}

		if (iNextIndex != iTopIndex)
		{
			_uint		iMinAquiredIndex_Next = { UINT_MAX };
			_uint		iMinAquiredIndex_Top = { UINT_MAX };
			_uint		iMaxAquiredIndex_Next = { 0 };
			_uint		iMaxAquiredIndex_Top = { 0 };

			for (_uint i = 0; i < static_cast<_uint>(AquiredSucceed[iTopIndex].size()); ++i)
			{
				if (true == AquiredSucceed[iTopIndex][i])
				{
					iMinAquiredIndex_Top = min(iMinAquiredIndex_Top, i);
					iMaxAquiredIndex_Top = max(iMaxAquiredIndex_Top, i);
				}
				if (true == AquiredSucceed[iNextIndex][i])
				{
					iMinAquiredIndex_Next = min(iMinAquiredIndex_Next, i);
					iMaxAquiredIndex_Next = max(iMaxAquiredIndex_Next, i);
				}
			}

			const _int		iDiffMin = { static_cast<_int>(iMinAquiredIndex_Next) - static_cast<_int>(iMinAquiredIndex_Top) };
			if (abs(iDiffMin) > ms_iMaxDiffAquired)
			{
				if (0 > iDiffMin)		//	음수 => Next가 Top 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMin) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iNextIndex][iMinAquiredIndex_Next + i] = false;
					Stack.push(iNextIndex);

				}
				else					//	양수 => Top이 Next 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMin) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iTopIndex][iMinAquiredIndex_Top + i] = false;
					Stack.push(iTopIndex);
				}
			}

			const _int		iDiffMax = { static_cast<_int>(iMaxAquiredIndex_Next) - static_cast<_int>(iMaxAquiredIndex_Top) };
			if (abs(iDiffMax) > ms_iMaxDiffAquired)
			{
				if (0 < iDiffMax)		//	양수 => Next가 Top 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMax) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iNextIndex][iMaxAquiredIndex_Next - i] = false;
					Stack.push(iNextIndex);

				}
				else					//	음수 => Top이 Next 보다 넓은 경우
				{
					const _uint		iNumPop = { abs(iDiffMax) - ms_iMaxDiffAquired };
					for (_uint i = 0; i < iNumPop; ++i)
						AquiredSucceed[iTopIndex][iMaxAquiredIndex_Top - i] = false;
					Stack.push(iTopIndex);
				}
			}
		}
	}


	//	요철 제거
	auto		iterPre = AquiredSucceed.end();
	auto		iterCur = AquiredSucceed.begin();
	auto		iterNext = AquiredSucceed.begin();	++iterNext;

	_uint		iMinAquiredIndex_Pre{ UINT_MAX }, iMaxAquiredIndex_Pre{ 0 };
	_uint		iMinAquiredIndex_Cur{ UINT_MAX }, iMaxAquiredIndex_Cur{ 0 };
	_uint		iMinAquiredIndex_Next{ UINT_MAX }, iMaxAquiredIndex_Next{ 0 };

	for (_uint i = 0; i < iterCur->size(); ++i)
	{
		if (true == (*iterCur)[i])
		{
			iMinAquiredIndex_Cur = min(iMinAquiredIndex_Cur, i);
			iMaxAquiredIndex_Cur = max(iMaxAquiredIndex_Cur, i);
		}
	}

	for (_uint i = 0; i < iterNext->size(); ++i)
	{
		if (true == (*iterNext)[i])
		{
			iMinAquiredIndex_Next = min(iMinAquiredIndex_Next, i);
			iMaxAquiredIndex_Next = max(iMaxAquiredIndex_Next, i);
		}
	}

	while (true)
	{
		iterPre = iterCur;
		iterCur = iterNext;
		++iterNext;

		if (iterNext == AquiredSucceed.end())
			break;

		iMinAquiredIndex_Pre = iMinAquiredIndex_Cur;
		iMaxAquiredIndex_Pre = iMaxAquiredIndex_Cur;

		iMinAquiredIndex_Cur = iMinAquiredIndex_Next;
		iMaxAquiredIndex_Cur = iMaxAquiredIndex_Next;

		iMinAquiredIndex_Next = UINT_MAX;
		iMaxAquiredIndex_Next = 0;

		for (_uint i = 0; i < iterNext->size(); ++i)
		{
			if (true == (*iterNext)[i])
			{
				iMinAquiredIndex_Next = min(iMinAquiredIndex_Next, i);
				iMaxAquiredIndex_Next = max(iMaxAquiredIndex_Next, i);
			}
		}

		if (iMinAquiredIndex_Next == iMinAquiredIndex_Pre &&
			iMinAquiredIndex_Cur < iMinAquiredIndex_Pre)
		{
			(*iterCur)[iMinAquiredIndex_Cur] = false;
		}

		if (iMaxAquiredIndex_Next == iMaxAquiredIndex_Pre &&
			iMaxAquiredIndex_Cur > iMaxAquiredIndex_Pre)
		{
			(*iterCur)[iMaxAquiredIndex_Cur] = false;
		}
	}


	return S_OK;
}

HRESULT CFormation_Manager::Optimize_Path_Duplication(list<_uint>& PathIndices)
{
	auto iterPre = PathIndices.end();
	auto iterCur = PathIndices.begin();
	auto iterNext = PathIndices.begin();		++iterNext;

	auto iterLast = PathIndices.end();			--iterLast;

	_uint		iNumErase{};

	while (iterLast != iterNext)
	{
		iterPre = iterCur;
		iterCur = iterNext;
		++iterNext;

		if ((*iterPre) == (*iterNext))
		{
			auto iterEraseStart = iterPre;
			auto iterEraseEnd = iterNext;
			++iterPre;
			++iterNext;

			while ((*iterPre) == (*iterNext))
			{
				iterEraseStart = iterPre;
				iterEraseEnd = iterNext;
				++iterPre;
				++iterNext;
			}

			iterEraseStart;
			++iterEraseEnd;

			while (iterEraseStart != iterEraseEnd)
			{
				iterEraseStart = PathIndices.erase(iterEraseStart);
				++iNumErase;
			}

			iterCur = iterEraseStart;
			--iterCur;
			iterNext = iterEraseStart;
		}
	}

	cout << "Optimize Path Duplication : FOrmation Manager Erase Cnt : " << iNumErase << endl;

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_For_Flocking(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex)
{
	if (FAILED(Initiate_Default_Param(iGroupIndexTag)))
		return E_FAIL;
	if (FAILED(Initiate_Formation_Virtual_Path_Flocking(iGroupIndexTag, iStartIndex, iGoalIndex)))
		return E_FAIL;
	if (FAILED(Initiate_Formation_Pathes(iGroupIndexTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFormation_Manager::Initiate_Formation_Virtual_Path_Flocking(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex)
{
	if (ms_iNumFormationGroupInfos <= iGroupIndexTag ||
		false == m_GroupInfos[iGroupIndexTag]->isActive)
		return E_FAIL;

	_uint				iNumAgent = { static_cast<_uint>(m_GroupInfos[iGroupIndexTag]->Formations.size()) };
	_uint				iFlowFieldIndexTag;
	list<_uint>			VirtualPathIndices;

	if (FAILED(m_pGameInstance->Make_VectorFiled_Daijkstra({ iGoalIndex }, iNumAgent, iFlowFieldIndexTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Convert_BestList_From_FlowField(VirtualPathIndices, iFlowFieldIndexTag, iStartIndex)))
		return E_FAIL;

	m_GroupInfos[iGroupIndexTag]->iFlowFieldIndexTag = iFlowFieldIndexTag;
	m_GroupInfos[iGroupIndexTag]->VirtualPath = move(VirtualPathIndices);

	return S_OK;
}

_float CFormation_Manager::Compute_Path_TotalDist(const list<_uint>& PathIndices)
{
	_float3		vVoxelCurPos;
	_float3		vVoxelPrePos;

	auto		iter = PathIndices.begin();
	_uint		iCurVoxelIndex = { *(iter) };
	if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vVoxelCurPos)))
		return -1.f;

	_float		fTotalDist = {};
	++iter;

	while (PathIndices.end() != iter)
	{
		vVoxelPrePos = vVoxelCurPos;
		iCurVoxelIndex = *(iter);
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iCurVoxelIndex, vVoxelCurPos)))
			return -1.f;

		_vector			vPrePos = { XMLoadFloat3(&vVoxelPrePos) };
		_vector			vCurPos = { XMLoadFloat3(&vVoxelCurPos) };

		_float			fDist = { XMVectorGetX(XMVector3Length(vCurPos - vPrePos)) };
		fTotalDist += fDist;
		++iter;
	}

	return fTotalDist;
}

CFormation_Manager* CFormation_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFormation_Manager* pInstance = { new CFormation_Manager(pDevice, pContext) };

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CFormation_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFormation_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (auto& pGroupInfo : m_GroupInfos)
	{
		for (auto& pFormation : pGroupInfo->Formations)
		{
			Safe_Release(pFormation);
			pFormation = nullptr;
		}
		pGroupInfo->Formations.clear();

		Safe_Delete(pGroupInfo);
		pGroupInfo = nullptr;
	}
	m_GroupInfos.clear();
}
