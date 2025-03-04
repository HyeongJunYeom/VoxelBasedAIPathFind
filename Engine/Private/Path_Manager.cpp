#include "Path_Manager.h"
#include "GameInstance.h"
#include "VIBuffer_Line.h"
#include "Transform.h"
#include "Navigation.h"

#include "PathFinder_FlowField.h"
#include "PathFinder_Voxel.h"

#include "VIBuffer_Instance_Line.h"

CPath_Manager::CPath_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPath_Manager::Initialize()
{
	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_PathFinder()))
		return E_FAIL;

	return S_OK;
}

void CPath_Manager::Priority_Tick(_float fTimeDelta)
{

}

void CPath_Manager::Tick(_float fTimeDelta)
{

}

HRESULT CPath_Manager::Render()
{
	for (auto& pObject : m_RenderObjects)
	{
		list<_float3>			BestRoute = { pObject->Get_BestPath_Positions_Float() };
		list<_float3>			OptimizedRoute = { pObject->Get_BestPath_Positions_Float_Optimized() };
		vector<_float3>			SmoothPath = { pObject->Get_BestPath_Positions_Smooth() };

		Render_Path(BestRoute, { 1.f, 0.f, 0.f, 1.f });
		Render_Path(OptimizedRoute, { 0.f, 0.f, 1.f, 1.f });
		Render_Round_Path(SmoothPath, { 1.f, 1.f, 1.f, 1.f });

		Safe_Release(pObject);
	}

	m_RenderObjects.clear();

	if (false == m_RenderPathes.empty())
	{
		Render_Path(m_RenderPathes.back(), { 1.f, 1.f, 1.f, 1.f });

		m_RenderPathes.clear();
	}


	vector<pair<_float3, _float3>>			Directions;
	const auto& FlowField{ m_pPathFinder_FlowField->Get_FlowField(m_iRenderFlowFieldIndex) };
	for (const auto& Pair : FlowField)
	{
		_uint			iVoxelIndex = { Pair.first };
		_float3			vDirectionFloat3 = { Pair.second };
		_float3			vPositionFloat3;

		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndex, vPositionFloat3)))
			continue;

		Directions.push_back({ vPositionFloat3, vDirectionFloat3 });
	}

	Render_Directions(Directions, { 1.f, 0.f, 0.f, 1.f });

	return S_OK;
}

HRESULT CPath_Manager::Add_Render_Path(class CNavigation* pNavigation)
{
	if (nullptr == pNavigation)
		return E_FAIL;

	Safe_AddRef(pNavigation);
	m_RenderObjects.push_back(pNavigation);

	return S_OK;
}

HRESULT CPath_Manager::Add_Render_Path(const list<_uint>& PathIndices)
{
	list<_float3>			PathPoses;
	for (auto iVoxelIndex : PathIndices)
	{
		_float3			vVoxelPos;
		if (FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndex, vVoxelPos)))
			return E_FAIL;

		PathPoses.push_back(vVoxelPos);
	}

	m_RenderPathes.push_back(move(PathPoses));
	return S_OK;
}

HRESULT CPath_Manager::Set_FlowFieldRenderIndex(const _uint iIndexTag)
{
	m_iRenderFlowFieldIndex = iIndexTag;

	return S_OK;
}

HRESULT CPath_Manager::Add_Components()
{
	m_pVIBuffer_Line = CVIBuffer_Line::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer_Line)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPoint_Line.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pVIBuffer_Instance_Line = CVIBuffer_Instance_Line::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer_Line)
		return E_FAIL;

	m_pShader_Instance_Line = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Line.hlsl"), VTXINSTANCE_LINE::Elements, VTXINSTANCE_LINE::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	return S_OK;
}

HRESULT CPath_Manager::Add_PathFinder()
{
	m_pPathFinder_FlowField = CPathFinder_FlowField::Create(m_pDevice, m_pContext);
	if (nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	m_pPathFinder_Voxel = CPathFinder_Voxel::Create(m_pDevice, m_pContext, nullptr);
	if (nullptr == m_pPathFinder_Voxel)
		return E_FAIL;

	return S_OK;
}

const unordered_map<_uint, _float3>& CPath_Manager::Get_FlowField(const _uint iIndexTag)
{
	static unordered_map<_uint, _float3> Temp;
	if (nullptr == m_pPathFinder_FlowField)
		return Temp;

	return m_pPathFinder_FlowField->Get_FlowField(iIndexTag);
}

void CPath_Manager::Render_Path(const list<_float3>& Path, _float4 vColor)
{
	if (true == Path.empty())
		return;

	_float3					vNextPos = { Path.front() };
	_float3					vLastPos;

	auto iter{ Path.begin() };
	++iter;

	while (iter != Path.end())
	{
		vLastPos = vNextPos;
		vNextPos = *iter;
		++iter;

		_float4x4		WorldMatrix = { m_pVIBuffer_Line->Compute_Look_At(XMLoadFloat3(&vLastPos), XMLoadFloat3(&vNextPos)) };

		_float			fOffset = { m_pGameInstance->Get_WorldSize_Voxel() * 0.5f };
		_matrix			OffsetTranslationMatrix = { XMMatrixTranslation(0.f, fOffset, 0.f) };
		_matrix			ResultMatrix = { XMLoadFloat4x4(&WorldMatrix) * OffsetTranslationMatrix };

		_float4x4		ResultFloat4x4;
		XMStoreFloat4x4(&ResultFloat4x4, ResultMatrix);
		

		if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &ResultFloat4x4)))
			return;
		if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return;
		if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return;
		if (FAILED(m_pShader->Bind_RawValue("g_vLineColor", &vColor, sizeof(_float4))))
			return;

		m_pShader->Begin(0);

		m_pVIBuffer_Line->Bind_Buffers();

		m_pVIBuffer_Line->Render();
	}
}

void CPath_Manager::Render_Round_Path(const vector<_float3>& Path, _float4 vColor)
{

	if (Path.empty())
		return;

	_vector		vPreSplinePos = {};
	_vector		vCurSplinePos = { XMLoadFloat3(&Path.front()) };
	for (_uint i = 1; i < Path.size(); ++i)
	{
		vPreSplinePos = vCurSplinePos;
		vCurSplinePos = XMLoadFloat3(&Path[i]);

		if (XMVectorGetX(XMVector3Length(vPreSplinePos - vCurSplinePos)) <= EPSILON)
			continue;

		_float4x4		WorldMatrix = { m_pVIBuffer_Line->Compute_Look_At(vPreSplinePos, vCurSplinePos) };
		if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
			return;
		if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return;
		if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return;
		if (FAILED(m_pShader->Bind_RawValue("g_vLineColor", &vColor, sizeof(_float4))))
			return;

		m_pShader->Begin(0);

		m_pVIBuffer_Line->Bind_Buffers();

		m_pVIBuffer_Line->Render();
	}

	return;
}

void CPath_Manager::Render_Directions(const vector<pair<_float3, _float3>>& Directions, const _float4& vColor)
{
	m_Instance_Poses_Buffer.clear();

	const _uint			iMaxInstance = { m_pVIBuffer_Instance_Line->Get_MaxInstance() };

	for(const auto& Pair : Directions)
	{
		const _float3&		vPosition = { Pair.first };
		const _float3&		vDirection = { Pair.second };


		_vector			vStart = { XMLoadFloat3(&vPosition) };
		_vector			vEnd = { vStart + XMLoadFloat3(&vDirection) * 0.25f };
		_float3			vStartFloat3, vEndFloat3;

		XMStoreFloat3(&vStartFloat3, vStart);
		XMStoreFloat3(&vEndFloat3, vEnd);

		m_Instance_Poses_Buffer.push_back({ vStartFloat3, vEndFloat3 });

		if (m_Instance_Poses_Buffer.size() >= iMaxInstance)
		{
			m_pVIBuffer_Instance_Line->Set_Positions(m_Instance_Poses_Buffer);

			//if (FAILED(m_pShader_Instance_Line->Bind_RawValue("g_vColor", &vColor, sizeof(_float4))))
			//	return;

			if (FAILED(m_pShader_Instance_Line->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
				return;
			if (FAILED(m_pShader_Instance_Line->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
				return;

			m_pShader_Instance_Line->Begin(0);
			m_pVIBuffer_Instance_Line->Bind_Buffers();
			m_pVIBuffer_Instance_Line->Render();

			m_Instance_Poses_Buffer.clear();
			m_Instance_Poses_Buffer.reserve(iMaxInstance);
		}
	}

	m_pVIBuffer_Instance_Line->Set_Positions(m_Instance_Poses_Buffer);

	/*if (FAILED(m_pShader_Instance_Line->Bind_RawValue("g_vColor", &vColor, sizeof(_float4))))
		return;*/

	if (FAILED(m_pShader_Instance_Line->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return;
	if (FAILED(m_pShader_Instance_Line->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return;

	m_pShader_Instance_Line->Begin(0);
	m_pVIBuffer_Instance_Line->Bind_Buffers();
	m_pVIBuffer_Instance_Line->Render();

	m_Instance_Poses_Buffer.clear();
	m_Instance_Poses_Buffer.reserve(iMaxInstance);
}

HRESULT CPath_Manager::Make_Path_Voxel(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices)
{
	if (nullptr == m_pPathFinder_Voxel)
		return E_FAIL;

	return m_pPathFinder_Voxel->Make_Path(PathIndices, NodeInfos, iStartIndex, GoalIndices, isDetourStaticObstacle, isDetourDynamicObstacle, MyReserveIndices);
}

HRESULT CPath_Manager::Make_Path_Inverse_Voxel(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle)
{
	if (nullptr == m_pPathFinder_Voxel)
		return E_FAIL;

	return m_pPathFinder_Voxel->Make_Path_Inverse(NodeInfos, StartIndices, iGoalIndex, isDetourStaticObstacle);

}

list<_float3> CPath_Manager::Get_BestPath_Positions_Float_Optimized()
{
	if (nullptr == m_pPathFinder_Voxel)
		return list<_float3>();

	return m_pPathFinder_Voxel->Get_BestPath_Positions_Float_Optimized();
}

const list<_uint>& CPath_Manager::Get_BestPath_Indices_Optimized()
{
	static list<_uint>			Temp;
	if (nullptr == m_pPathFinder_Voxel)
		return Temp;

	return m_pPathFinder_Voxel->Get_BestPath_Indices_Optimized();
}

const vector<_float3>& CPath_Manager::Get_BestPath_Positions_Smooth()
{
	static vector<_float3>			Temp;
	if (nullptr == m_pPathFinder_Voxel)
		return Temp;

	return m_pPathFinder_Voxel->Get_BestPath_Positions_Smooth();
}

HRESULT CPath_Manager::Clear_PathInfos()
{
	if (nullptr == m_pPathFinder_Voxel)
		return E_FAIL;

	return m_pPathFinder_Voxel->Clear_PathInfos();
}

const unordered_set<_uint>& CPath_Manager::Get_OpenList()
{
	static unordered_set<_uint>			Temp;
	if (nullptr == m_pPathFinder_Voxel)
		return Temp;

	return m_pPathFinder_Voxel->Get_OpenList();
}

const unordered_set<_uint>& CPath_Manager::Get_CloseList()
{
	static unordered_set<_uint>			Temp;
	if (nullptr == m_pPathFinder_Voxel)
		return Temp;

	return m_pPathFinder_Voxel->Get_CloseList();
}

ALGORITHM CPath_Manager::Get_Algorithm()
{
	if (nullptr == m_pPathFinder_Voxel)
		return ALGORITHM::_END;

	return m_pPathFinder_Voxel->Get_Algorithm();
}

MAIN_HEURISTIC CPath_Manager::Get_MainHeuristic()
{
	if (nullptr == m_pPathFinder_Voxel)
		return MAIN_HEURISTIC::_END;

	return m_pPathFinder_Voxel->Get_MainHeuristic();
}

void CPath_Manager::Set_Algorithm(ALGORITHM eAlgorithm)
{
	if (nullptr == m_pPathFinder_Voxel)
		return;

	return m_pPathFinder_Voxel->Set_Algorithm(eAlgorithm);
}

void CPath_Manager::Set_MainHeuristic(MAIN_HEURISTIC eHeuristic)
{
	if (nullptr == m_pPathFinder_Voxel)
		return;

	return m_pPathFinder_Voxel->Set_MainHeuristic(eHeuristic);
}

void CPath_Manager::Set_MaxCost(const _float fMaxCost)
{
	if (nullptr == m_pPathFinder_Voxel)
		return;

	return m_pPathFinder_Voxel->Set_MaxCost(fMaxCost);
}

TEST_MODE CPath_Manager::Get_OptimizerTestMode()
{
	if (nullptr == m_pPathFinder_Voxel)
		return TEST_MODE::_END;

	return m_pPathFinder_Voxel->Get_OptimizerTestMode();
}

void CPath_Manager::Set_OptimizerTestMode(TEST_MODE eTestMode)
{
	if (nullptr == m_pPathFinder_Voxel)
		return;

	return m_pPathFinder_Voxel->Set_OptimizerTestMode(eTestMode);
}

_float CPath_Manager::Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType)
{
	if (nullptr == m_pPathFinder_Voxel)
		return 0.f;

	return m_pPathFinder_Voxel->Get_HeuristicWeights(eHeuriticType);
}

void CPath_Manager::Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight)
{
	if (nullptr == m_pPathFinder_Voxel)
		return;

	return m_pPathFinder_Voxel->Set_HeuristicWeights(eHeuriticType, fWeight);
}

_float CPath_Manager::Compute_Total_Dist_BestPath(const list<_uint>& PathIndices)
{
	if (nullptr == m_pPathFinder_Voxel)
		return 0.f;

	return m_pPathFinder_Voxel->Compute_Total_Dist_BestPath(PathIndices);
}

_float CPath_Manager::Compute_Total_Dist_OptimizedPath()
{
	if (nullptr == m_pPathFinder_Voxel)
		return 0.f;

	return m_pPathFinder_Voxel->Compute_Total_Dist_OptimizedPath();
}

_llong CPath_Manager::Get_Make_Path_Time()
{
	if (nullptr == m_pPathFinder_Voxel)
		return _llong();

	return m_pPathFinder_Voxel->Get_Make_Path_Time();
}

_llong CPath_Manager::Get_Optimize_Path_Time()
{
	if (nullptr == m_pPathFinder_Voxel)
		return _llong();

	return m_pPathFinder_Voxel->Get_Optimize_Path_Time();
}

_llong CPath_Manager::Get_Smooth_Path_Time()
{
	if (nullptr == m_pPathFinder_Voxel)
		return _llong();

	return m_pPathFinder_Voxel->Get_Smooth_Path_Time();
}

HRESULT CPath_Manager::Make_VectorFiled_Daijkstra(const vector<_uint>& GoalIndices, const _uint iNumStartAgent, _uint& iIndexTag)
{
	if (nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	return m_pPathFinder_FlowField->Make_VectorFiled_Daijkstra(GoalIndices, iNumStartAgent, iIndexTag);
}

HRESULT CPath_Manager::Make_VectorFiled_A_Star(const _uint iGoalIndex, const vector<_uint>& StartIndices, const _uint iNumStartAgent, _uint& iIndexTag)
{
	if (nullptr == m_pPathFinder_FlowField ||
		nullptr == m_pPathFinder_Voxel)
		return E_FAIL;

	NODE_INFOS			NodeInfos;

	if (FAILED(m_pPathFinder_Voxel->Make_Path_Inverse(NodeInfos, StartIndices, iGoalIndex, true)))
		return E_FAIL;

	return m_pPathFinder_FlowField->Make_VectorFiled_A_Star(StartIndices, iGoalIndex, NodeInfos, iNumStartAgent, iIndexTag);
}

HRESULT CPath_Manager::Get_Direction_FlowField(const _float3& vCurPos, const _uint iIndexTag, _float3& vDirection)
{
	if (nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	return m_pPathFinder_FlowField->Get_Direction_FlowField(vCurPos, iIndexTag, vDirection);
}

HRESULT CPath_Manager::Get_ParentIndex_FlowField(const _float3& vCurPos, const _uint iIndexTag, _uint& iParentIndex)
{
	if (nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	return m_pPathFinder_FlowField->Get_ParentIndex_FlowField(vCurPos, iIndexTag, iParentIndex);
}

_float CPath_Manager::Get_Distance_From_Goal_FlowField(const _uint iVoxelIndex, const _uint iIndexTag)
{
	if (nullptr == m_pPathFinder_FlowField)
		return FLT_MAX;

	return m_pPathFinder_FlowField->Get_DistanceFromGoal(iVoxelIndex, iIndexTag);
}

HRESULT CPath_Manager::AddRef_FlowField(const _uint iIndexTag)
{
	if(nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	return m_pPathFinder_FlowField->AddRef_FlowField(iIndexTag);
}

HRESULT CPath_Manager::Release_FlowField(const _uint iIndexTag)
{
	if (nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	return m_pPathFinder_FlowField->Release_FlowField(iIndexTag);
}

vector<_uint> CPath_Manager::Get_Active_FlowField_RefCnt()
{
	if (nullptr == m_pPathFinder_FlowField)
		return vector<_uint>();

	return m_pPathFinder_FlowField->Get_Active_FlowField_RefCnt();
}

vector<_uint> CPath_Manager::Get_Active_FlowField_Indices()
{
	if (nullptr == m_pPathFinder_FlowField)
		return vector<_uint>();

	return m_pPathFinder_FlowField->Get_Active_FlowField_Indices();
}

HRESULT CPath_Manager::Convert_BestList_From_FlowField(list<_uint>& BestList, const _uint iIndexTag, const _uint iStartIndex)
{
	if (nullptr == m_pPathFinder_FlowField)
		return E_FAIL;

	_uint			iCurIndex = { iStartIndex };
	_uint			iParentIndex;
	if (FAILED(m_pPathFinder_FlowField->Get_ParentIndex_FlowField(iCurIndex, iIndexTag, iParentIndex)))
		return E_FAIL;

	BestList.push_back(iCurIndex);

	while (iParentIndex != iCurIndex)
	{
		iCurIndex = iParentIndex;
		BestList.push_back(iCurIndex);

		if (FAILED(m_pPathFinder_FlowField->Get_ParentIndex_FlowField(iCurIndex, iIndexTag, iParentIndex)))
			break;
	}	

	return S_OK;
}

vector<_float3> CPath_Manager::Make_SmoothPath(_fvector vStart, _fvector vEnd, _fvector vDir, _float fRadius, _uint iNumSample)
{
	vector<_float3> SmoothPath;

	_vector			vDirToEnd = { vEnd - vStart };
	_float			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDirToEnd), XMVector3Normalize(vDir))) };

	_float3				vTemp;
	XMStoreFloat3(&vTemp, vStart);
	SmoothPath.push_back(vTemp);

	if (fDot <= 1.f - EPSILON)
	{
		_vector			vAxis = { XMVector3Normalize(XMVector3Cross(XMVector3Normalize(vDir), XMVector3Normalize(vDirToEnd))) };
		_vector			vToCircleCenterDir = { XMVector3Normalize(XMVector3Cross(XMVector3Normalize(vAxis), XMVector3Normalize(vDir))) };
		_vector			vCircleCenterPos = { vStart + (vToCircleCenterDir * fRadius) };

		//	//	_float			fDist = { XMVectorGetX(XMVector3Length(vDirToEnd)) };
		_vector			vNextPos = { vStart };
		_vector			vDirToNext = { vNextPos - vCircleCenterPos };
		_float			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDirToNext), XMVector3Normalize(vEnd - vNextPos))) };
		_float			fAngle = { 0.f };
		_float			fAnlgeStep = { 5.f * XM_PI / 180.f };
		_matrix			RotateMatrix = { XMMatrixRotationAxis(vAxis, fAnlgeStep) };
		_matrix			RotateMatrix2 = { XMMatrixRotationAxis(vAxis, XMConvertToRadians(90.f)) };

		_float			fPreDot = { fDot };
		while (true)

		{
			vDirToNext = XMVector3TransformNormal(vDirToNext, RotateMatrix);
			vNextPos = vCircleCenterPos + vDirToNext;

			_vector			vCurDir = { XMVector3TransformNormal(vDirToNext, RotateMatrix2) };

			fPreDot = fDot;
			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vCurDir), XMVector3Normalize(vEnd - vNextPos))) };

			if(fPreDot > fDot)
				break;

			XMStoreFloat3(&vTemp, vNextPos);
			SmoothPath.push_back(vTemp);
		}
	}

	XMStoreFloat3(&vTemp, vEnd);
	SmoothPath.push_back(vTemp);

	return SmoothPath;
}

CPath_Manager* CPath_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPath_Manager* pInstance = { new CPath_Manager{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CPath_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPath_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pVIBuffer_Line);
	Safe_Release(m_pShader);

	Safe_Release(m_pVIBuffer_Instance_Line);
	Safe_Release(m_pShader_Instance_Line);

	Safe_Release(m_pPathFinder_FlowField);
	Safe_Release(m_pPathFinder_Voxel);

	for (auto& pObjects : m_RenderObjects)
		Safe_Release(pObjects);
}
